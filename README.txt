# PList

## **Rendu**

jonathan.tondelier@ecole2600.com : 50%
boris.trouche@ecole2600.com : 50%


<br>
<br>


## **Installation**

```bash
cl *.c -o plist.exe /link advapi32.lib pdh.lib
```


<br>
<br>


## **Sujet:**
  
PList (Process List) est un utilitaire ligne de commande qui affiche les processus en cours
d’exécution sur l’ordinateur local, ainsi que des informations utiles sur chaque processus.

PList affiche:
- Les processus en cours d’exécution sur l’ordinateur, ainsi que leurs ID de processus
  (PID).

- Détails du processus: utilisation de la mémoire virtuelle.

- Threads en cours d’exécution dans chaque processus, y compris leurs ID de thread.

- En optionnel, Détails du processus: la commande dont il émane, Détails du thread:
  points d’entrée, la dernière erreur signalée, état du thread.
