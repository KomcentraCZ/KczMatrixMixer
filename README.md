ROMAN'S NOTES:
==============
DATE 250505:
------------
- stream app: vychazi z examplu
https://docs.pipewire.org/tutorial4_8c-example.html , na tom jsem si
rozjizdel prostredi a overoval jestli mi to jede;

- matrix mixer: vychazi z tohoto
https://docs.pipewire.org/audio-dsp-filter_8c-example.html , ma 16x16
kanalu a v sobe logiku maticoveho mixaku. Konfigurace je v tom headeru
vedle. Po spusteni se hlasi v PipeWire zadanymi nazvy portu. Matice je
ted nastavena tak, ze vstup 1 jde na vystup 1 atd.

co dal:
- musime se domluvit, jake udelame rozhrani (extern C, I2C .. ?)
- jakmile se matice zacne nastavovat dynamicky, budu muset jeste
dopsat interpolaci parametru, aby to pri zmene volume nepraskalo
- mohu udelat prototyp programoveho propojovani portu. Da se pres
pipewire api, pw-cli a nebo pres lua scripty ve wireplumberu
- testovani

- autogain, expander ...
