/* PipeWire */
/* SPDX-FileCopyrightText: Copyright © 2019 Wim Taymans */
/* SPDX-License-Identifier: MIT */
 
/*
 [title]
 Audio filter using \ref pw_filter "pw_filter".
 [title]
 */
 
 #include <stdio.h>
 #include <errno.h>
 #include <math.h>
 #include <signal.h>
  
 #include <spa/pod/builder.h>
 #include <spa/param/latency-utils.h>
  
 #include <pipewire/pipewire.h>
 #include <pipewire/filter.h>

 #include "matrix_settings.h"
  
 struct data;
  
 struct port {
         struct data* data;
 };
  
 struct data {
         struct pw_main_loop* loop;
         struct pw_filter* filter;
         struct port* in_port[NUM_CHANNELS];
         struct port* out_port[NUM_CHANNELS];
 };
  
 /* our data processing function is in general:
  *
  *  struct pw_buffer *b;
  *  in = pw_filter_dequeue_buffer(filter, in_port);
  *  out = pw_filter_dequeue_buffer(filter, out_port);
  *
  *  .. do stuff with buffers ...
  *
  *  pw_filter_queue_buffer(filter, in_port, in);
  *  pw_filter_queue_buffer(filter, out_port, out);
  *
  *  For DSP ports, there is a shortcut to directly dequeue, get
  *  the data and requeue the buffer with pw_filter_get_dsp_buffer().
  *
  *
  */
 static void on_process(void* userdata, struct spa_io_position *position)
 {
         struct data *data = (struct data*) userdata;
         float* in[NUM_CHANNELS];
         float* out[NUM_CHANNELS];
         uint32_t n_samples = position->clock.duration;
  
         pw_log_trace("do process %d", n_samples);

         for (size_t i = 0; i < NUM_CHANNELS; i++) {
                in[i] = (float*)pw_filter_get_dsp_buffer(data->in_port[i], n_samples);
                out[i] = (float*)pw_filter_get_dsp_buffer(data->out_port[i], n_samples);
                
                if (in[i] == NULL || out[i] == NULL)
                        continue;
                
                memset(out[i], 0, n_samples*sizeof(float));
         }

         for (size_t i = 0; i < NUM_CHANNELS; i++) { // inputs
                if (in[i] == NULL) continue;
                for (size_t j = 0; j < NUM_CHANNELS; j++) { // outputs
                        if (out[j] == NULL) continue;
                        for(size_t k = 0; k < n_samples; k++) {
                                out[j][k] += in[i][k] * volume_matrix[i][j];
                        }
                }
         }
 }
  
 static const struct pw_filter_events filter_events = {
         PW_VERSION_FILTER_EVENTS,
         .process = on_process,
 };
  
 static void do_quit(void *userdata, int signal_number)
 {
         struct data *data = (struct data*) userdata;
         pw_main_loop_quit(data->loop);
 }
  
 int main(int argc, char *argv[])
 {
         struct data data = { 0, };
         const struct spa_pod *params[1];
         uint8_t buffer[1024];
         struct spa_pod_builder b = SPA_POD_BUILDER_INIT(buffer, sizeof(buffer));
  
         pw_init(&argc, &argv);
  
         /* make a main loop. If you already have another main loop, you can add
          * the fd of this pipewire mainloop to it. */
         data.loop = pw_main_loop_new(NULL);
  
         pw_loop_add_signal(pw_main_loop_get_loop(data.loop), SIGINT, do_quit, &data);
         pw_loop_add_signal(pw_main_loop_get_loop(data.loop), SIGTERM, do_quit, &data);
  
         /* Create a simple filter, the simple filter manages the core and remote
          * objects for you if you don't need to deal with them.
          *
          * Pass your events and a user_data pointer as the last arguments. This
          * will inform you about the filter state. The most important event
          * you need to listen to is the process event where you need to process
          * the data.
          */
         data.filter = pw_filter_new_simple(
                         pw_main_loop_get_loop(data.loop),
                         "audio-filter",
                         pw_properties_new(
                                 PW_KEY_MEDIA_TYPE, "Audio",
                                 PW_KEY_MEDIA_CATEGORY, "Filter",
                                 PW_KEY_MEDIA_ROLE, "DSP",
                                 NULL),
                         &filter_events,
                         &data);


         for (size_t i = 0; i < NUM_CHANNELS; i++) {
                data.in_port[i] = (port*)pw_filter_add_port(data.filter,
                        PW_DIRECTION_INPUT,
                        PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                        sizeof(struct port),
                        pw_properties_new(
                                PW_KEY_FORMAT_DSP, "32 bit float mono audio",
                                PW_KEY_PORT_NAME, channel_names[0][i],
                                NULL),
                        NULL, 0);

                                 
                data.out_port[i] = (port*)pw_filter_add_port(data.filter,
                        PW_DIRECTION_OUTPUT,
                        PW_FILTER_PORT_FLAG_MAP_BUFFERS,
                        sizeof(struct port),
                        pw_properties_new(
                                PW_KEY_FORMAT_DSP, "32 bit float mono audio",
                                PW_KEY_PORT_NAME, channel_names[1][i],
                                NULL),
                        NULL, 0);
         }
  
        struct spa_process_latency_info latency_info { .ns = 10 * (1000000ll) };
        params[0] = spa_process_latency_build(&b,
                         SPA_PARAM_ProcessLatency,
                         &latency_info);
  
  
         /* Now connect this filter. We ask that our process function is
          * called in a realtime thread. */
         if (pw_filter_connect(data.filter,
                                 PW_FILTER_FLAG_RT_PROCESS,
                                 params, 1) < 0) {
                 fprintf(stderr, "can't connect\n");
                 return -1;
         }
  
         /* and wait while we let things run */
         pw_main_loop_run(data.loop);
  
         pw_filter_destroy(data.filter);
         pw_main_loop_destroy(data.loop);
         pw_deinit();
  
         return 0;
 }