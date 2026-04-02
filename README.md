# Hello World Example

1. Installa VS Code
2. installa l'estensione: ESP-IDF
3. Sul pannello alla tua sinistra è apparso un tab con il logo
4. Via su Logo -> advanced -> Configure ESP-IDF Extension
   1. Gihub
   2. v5.4.1
   3. Il resto va bene così
5. installa tutto e riavvia per sicurezza, perchè i PATH si aggiornano
6. Apri le impostazioni di VS Code con `CTRL + ,`
7. apri il file di configurazione con il bottone in alto a destra `Apri impostazioni JSON`
8. Copia le seguenti righe inserendole nel file:

    ```json
    "terminal.integrated.defaultProfile.windows": "PowerShell",
    "terminal.integrated.shellIntegration.enabled": true,
    ```

9. Apri il progetto
10. in basso appaiono diversi bottoni per flashare, compilare etc...
11. Ti consiglio come prima cosa di ripulire la build per sicurezza (trash can)
12. configura prima la VERSIONE di ESP-IDF v5.4.1
13. Configura l'interfaccia di FLASH UART
14. Configura la porta (a me risulta COM7 ma sarà diversa chiaramente)
15. Seleziona il CHIP esp32s3
16. Ora puoi con il bottone del fuoco: COMPILARE, FLASHARE e inizializzare il MONITOR SERIALE
17. Fatto :smile:

P.S.

> Consiglio vivamente di installare altre estensioni utili come:
>
> - clang-format
> - Markdown All in One
> - C/C++ (Microsoft)
> - CMake
> - CMake Tools
