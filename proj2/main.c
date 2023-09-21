/*
 * Soubor:  proj2.c
 * Projekt: IOS-Projekt 2
 * Datum:   26.4.2017
 * Autor:   FrantiĹĄek HorĂĄznĂ˝ (xhoraz02) - FIT VUT
 * PĂ¸eklad: gcc 4.3.2
 * Popis: Implementujte v jazyce C modifikovanĂ˝ synchronizaÄnĂ­ problĂŠm Child Care.
 * Existuje centrum, kterĂŠ se starĂĄ o dÄti, a dva typy procesĹŻ: dospÄlĂ˝ ÄlovÄk (adult) a dĂ­tÄ (child).
 * V centru se dospÄlĂ­ lidĂŠ starajĂ­ o dÄti, pĹiÄemĹž jeden dospÄlĂ˝ se mĹŻĹže starat nejvĂ˝ĹĄe o tĹi dÄti.
 * NapĹ. pokud je v centru 5 dÄtĂ­, musĂ­ bĂ˝t pĹĂ­tomni alespoĹ 2 dospÄlĂ­, pokud jsou pouze 3 dÄti, staÄĂ­ jeden dospÄlĂ˝.
 * Do centra pĹichĂĄzejĂ­ a z centra odchĂĄzejĂ­ dospÄlĂ­ lidĂŠ a dÄti, ovĹĄem tak, aby nebyla poruĹĄena vĂ˝ĹĄe uvedenĂĄ vĂ˝jimka.
 * Pokud by byla odchodem dospÄlĂŠho poruĹĄena podmĂ­nka, musĂ­ s odchodem vyÄkat, neĹž odejde dostateÄnĂ˝ poÄet dÄtĂ­.
 * PodobnÄ, pokud chce vstoupit dĂ­tÄ a v centru nenĂ­ dostatek dospÄlĂ˝ch, musĂ­ vyÄkat, aĹž pĹĂ­jde dalĹĄĂ­ dospÄlĂ˝.
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <semaphore.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


const char *shmname = "xhoraz02";




typedef struct sharemem
{
    int poradoveCislo;
    int ChildCount;
    int AdultCount;
    int adultEnded;
    int childEnded;
    int cekajici;
    int odchazejici;
    sem_t vstupSem;
    sem_t konec;
    sem_t vstupChild;
    sem_t vystupAdult;
} ShareMem;

/** globalni promenna pro sdilenou pamet */
ShareMem *shared;


int value;

FILE * zapis;

void kill_all(pid_t pids[]);
void child_generator(long int C,long int CGT,long int CWT, long int A);
void adult_generator(long int A,long int AGT,long int AWT,long int C);
void child(long int CWT,int i, long int A, long int C);
void adult(long int AWT,int i, long int A, long int C);



/*
 * Funkce: main
 * Params: 1) PoĂ¨et parametrĂš pĂ¸Ă­kazovĂŠho Ă¸ĂĄdku 2) Pole se parametry pĂ¸Ă­kazovĂŠho Ă¸ĂĄdku
 * VracĂ­:  Pokud program probĂŹhne ĂşspĂŹĹĄnĂŹ, vracĂ­ 0
 * Popis:  Funkce nejdĂ¸Ă­ve zkontroluje zadanĂŠ parametry pĂ¸Ă­kazovĂŠho Ă¸ĂĄdku, potĂŠ zinicializuje potĂ¸ebnĂŠ prostĂ¸edky a vytvoĂ¸Ă­ argumentem zadanĂ˝
 *         poĂ¨et potomkĂš, jejichĹž Ă¸Ă­zenĂ­ pĂ¸edĂĄ funkci child, Ă¸Ă­zenĂ­ rodiĂ¨ovskĂŠho procesu pĂ¸edĂĄ funkci adult.
 */
int main(int argc, char *argv[])
{
    // Kontrola poĂ¨tu parametrĂš - program oĂ¨ekĂĄvĂĄ jeden zadanĂ˝ parametr + "nultĂ˝" je jmĂŠno spuĹĄtĂŹnĂŠho souboru
    if(argc != 7)
        {
            fprintf(stderr,"Incorrect number of parameters!\nExpected:6 numbers with space between\nA C AGT CGT AWT CWT");
            exit(2);
        }

    char *kontrola;
    long int A,C,AGT,CGT,AWT,CWT;
    if (argc==7)
    {
    A = strtol(argv[1],&kontrola,10);
    if (kontrola[0]!='\0'||A<1){
        fprintf(stderr,"chyba v nacitani 1. arg");
        exit(2);
        }
    C = strtol(argv[2],&kontrola,10);
    if (kontrola[0]!='\0'||C<1){
        fprintf(stderr,"chyba v nacitani 2. arg");
        exit(2);
        }
    AGT = strtol(argv[3],&kontrola,10);
    if (kontrola[0]!='\0'||AGT<0||AGT>5000){
        fprintf(stderr,"chyba v nacitani 3. arg");
        exit(2);
        }
    CGT = strtol(argv[4],&kontrola,10);
    if (kontrola[0]!='\0'||CGT<0||CGT>5000){
        fprintf(stderr,"chyba v nacitani 4. arg");
        exit(2);
        }
    AWT = strtol(argv[5],&kontrola,10);
    if (kontrola[0]!='\0'||AWT<0||AWT>5000){
        fprintf(stderr,"chyba v nacitani 5. arg");
        exit(2);
        }
    CWT = strtol(argv[6],&kontrola,10);
    if (kontrola[0]!='\0'||CWT<0||CWT>5000){
        fprintf(stderr,"chyba v nacitani 6. arg");
        exit(2);
        }
    }
    zapis = fopen("proj2.out","w");
    if (zapis==NULL)
    {
        fprintf(stderr,"chyba v otevirani souboru pro zapis\n");
        exit(2);
    }
    setbuf(zapis, NULL);

    srand(time(NULL));
    int fileDescriptor;
    fileDescriptor = shm_open(shmname, O_RDWR | O_CREAT, 0600);

    shared = mmap(NULL, sizeof (ShareMem), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, fileDescriptor , 0);
    shared->ChildCount=0;
    shared->AdultCount=0;
    shared->cekajici=0;
    shared->odchazejici=0;
    shared->poradoveCislo=0;
    shared->adultEnded=0;
    shared->childEnded=0;
    sem_init(&(shared->vystupAdult), 1, 0);
    sem_init(&(shared->vstupChild), 1, 0);
    sem_init(&(shared->vstupSem), 1, 1);
    sem_init(&(shared->konec), 1, 0);

    pid_t pid;
    pid_t CGpid;
    pid_t AGpid;

       // systemove volani - vzdy je vhodne overit uspesnost!
    if ((pid = fork()) < 0)
        {
        perror("fork");
        exit(2);
        }

    if (pid == 0)  // child od mainu (adult generator)
        {
        adult_generator(A,AGT,AWT,C);
        exit(0);
        }
    else
        { // parent (main)
        AGpid=pid;
        if ((pid = fork()) < 0)
            {
            perror("fork");
            exit(2);
            }

        if (pid == 0)  // child od mainu (child generator)
        {
        child_generator(C,CGT,CWT,A);
        exit(0);
        }
     else
        { // parent (main)
            CGpid=pid;
            waitpid(CGpid,NULL,0);
            waitpid(AGpid,NULL,0);
        }
    }

    fclose(zapis);
    shm_unlink(shmname);
    return 0;
}

void child_generator (long int C,long int CGT,long int CWT, long int A)
{
    int pid;
    pid_t pids[(C+1)];
    memset(pids,0,(C+1)*sizeof(pid_t));
    for(int i = 0; i < C; i++)
    {
        usleep((rand()%CGT)*1000);
        // Pokus o vytvoĂ¸enĂ­ potomka
        pid = fork();
        // VytvoĂ¸enĂ­ potomka se nezdaĂ¸ilo
        if(pid == -1)
        {
            // UkonĂ¨enĂ­ dĂ¸Ă­ve vytvorenĂ˝ch potomkĂš
            kill_all(pids);
            exit(2);
        }

        // VytvorenĂ­ potomka se zdarilo, nynĂ­ je treba rozliĹĄit, kterĂ˝ proces je potomek a kterĂ˝ rodic
        if(pid == 0)
            // ObsluĹžna funkce pro proces zakaznĂ­ka (potomek), ktera se nikdy navratĂ­
            child(CWT,i,A,C);
        else
            // UloĹženĂ­ PID potomka
            pids[i] = pid;
    }
    for (int k = 0; k < (C+1); k++)
        {
        waitpid(pids[k], NULL, 0);
        }

}
/**
* generuje procesy a uklĂĄdĂĄ do pole jejich pid
*   pote skonci
* 1. param konecny pocet dospelych
* 2. param doba
*/
void adult_generator (long int A,long int AGT,long int AWT, long int C)
{
    int pid;
    pid_t pids[(A+1)];
    memset(pids,0,(A+1)*sizeof(pid_t));
    for(int i = 0; i < A; i++)
    {
        usleep((rand()%AGT)*1000);
        // Pokus o vytvoĂ¸enĂ­ potomka
        pid = fork();
        // VytvoĂ¸enĂ­ potomka se nezdaĂ¸ilo
        if(pid == -1)
        {
            // UkonĂ¨enĂ­ dĂ¸Ă­ve vytvoĂ¸enĂ˝ch potomkĂš
            kill_all(pids);
            exit(2);
        }

        // VytvoĂ¸enĂ­ potomka se zdaĂ¸ilo, nynĂ­ je tĂ¸eba rozliĹĄit, kterĂ˝ proces je potomek a kterĂ˝ rodiĂ¨
        if(pid == 0)
            // ObsluĹžnĂĄ funkce pro proces zĂĄkaznĂ­ka (potomek), kterĂĄ se nikdy navrĂĄtĂ­
            adult(AWT,i,A,C);
        else
            // UloĹženĂ­ PID potomka
            pids[i] = pid;
    }
    for (int k = 0; k < (A+1); k++)
        {
        waitpid(pids[k], NULL, 0);
        }
}

/**
* obsluzna funkce pro deti
* vypisuje a pouziva semafory pro cekani popripade pousteni dovnitr
* parametr maximalni doba spani
* parametr kolikaty proces to je
*/
void child(long int CWT,int i, long int A, long int C)
{   i++;
    sem_wait(&shared->vstupSem);
    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: C %-4i: started\n",shared->poradoveCislo,i);
        if (A==shared->adultEnded&&shared->cekajici>0)
        {
            while (shared->cekajici>0)
            {
                sem_post(&shared->vstupChild);
                shared->cekajici--;
            }
        }
        if (shared->ChildCount<3*shared->AdultCount||A==shared->adultEnded)
        {
            shared->ChildCount++;
            shared->poradoveCislo++;
            fprintf(zapis,"%-10i: C %-4i: enter\n",shared->poradoveCislo,i);
        sem_post(&shared->vstupSem);
        }
        else
        {
            shared->cekajici++;
            shared->poradoveCislo++;
            fprintf(zapis,"%-10i: C %-4i: waiting : %-2i: %-2i\n",shared->poradoveCislo,i,shared->AdultCount,shared->ChildCount);
            sem_post(&shared->vstupSem);
            sem_wait(&shared->vstupChild);
            shared->poradoveCislo++;
            fprintf(zapis,"%-10i: C %-4i: enter\n",shared->poradoveCislo,i);
        }

        usleep((rand()%CWT)*1000);

        sem_wait(&shared->vstupSem);
        shared->ChildCount--;
        shared->poradoveCislo++;
        fprintf(zapis,"%-10i: C %-4i: trying to leave\n",shared->poradoveCislo,i);
        shared->childEnded++;
        shared->poradoveCislo++;
        fprintf(zapis,"%-10i: C %-4i: leave\n",shared->poradoveCislo,i);

        if (shared->odchazejici>0&&shared->ChildCount<=3*(shared->AdultCount-1))
        {
            shared->odchazejici--;
            shared->AdultCount--;
            sem_post(&shared->vystupAdult);
        }
    sem_post(&shared->vstupSem);
   /**
    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: C %-4i: waiting : %-2i: %-2i\n",shared->poradoveCislo,i,shared->AdultCount,shared->ChildCount);

    shared->ChildCount++;
    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: C %-4i: enter\n",shared->poradoveCislo,i);

    shared->ChildCount++;
    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: C %-4i: enter\n",shared->poradoveCislo,i);

    usleep((rand()%CWT)*1000);

    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: C %-4i: trying to leave\n",shared->poradoveCislo,i);

    shared->childEnded++;
    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: C %-4i: leave\n",shared->poradoveCislo,i);

*/
    if (shared->adultEnded==A&&shared->childEnded==C)
        sem_post(&shared->konec);
    sem_wait(&shared->konec);
    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: C %-4i: finished\n",shared->poradoveCislo,i);
    sem_post(&shared->konec);
    exit(0);

}
/**
* obsluzna funkce pro dospele
* vypisuje a pouziva semafory pro cekani popripade pousteni dovnitr
* parametr maximalni doba spani
* parametr kolikaty proces to je
*/
void adult(long int AWT,int i, long int A, long int C)
{   i++;
    sem_wait(&shared->vstupSem);
        shared->AdultCount++;
        shared->poradoveCislo++;
        fprintf(zapis,"%-10i: A %-4i: started\n",shared->poradoveCislo,i);
        shared->poradoveCislo++;
        fprintf(zapis,"%-10i: A %-4i: enter\n",shared->poradoveCislo,i);

        if (shared->cekajici>0)
        {
            int n=3;
            if (shared->cekajici<4)
                n=shared->cekajici;
            for (int k=n;k>0;k--)
                sem_post(&shared->vstupChild);
            shared->cekajici -= n;
            shared->ChildCount += n;
        }

    sem_post(&shared->vstupSem);

    usleep((rand()%AWT)*1000);

    sem_wait(&shared->vstupSem);
    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: A %-4i: trying to leave\n",shared->poradoveCislo,i);

        if (shared->ChildCount <= 3*(shared->AdultCount-1))
        {
            shared->AdultCount--;
            shared->poradoveCislo++;
            shared->adultEnded++;
            fprintf(zapis,"%-10i: A %-4i: leave\n",shared->poradoveCislo,i);
            sem_post(&shared->vstupSem);
        }
        else
        {
            shared->odchazejici++;
            shared->poradoveCislo++;
            fprintf(zapis,"%-10i: A %-4i: waiting : %-2i: %-2i\n",shared->poradoveCislo,i,shared->AdultCount,shared->ChildCount);
            sem_post(&shared->vstupSem);

            sem_wait(&shared->vystupAdult);
            sem_wait(&shared->vstupSem);
            shared->adultEnded++;
            shared->poradoveCislo++;
            fprintf(zapis,"%-10i: A %-4i: leave\n",shared->poradoveCislo,i);
            sem_post(&shared->vstupSem);
        }

/**
    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: A %-4i: started\n",shared->poradoveCislo,i);
    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: A %-4i: enter\n",shared->poradoveCislo,i);

    usleep((rand()%AWT)*1000);

    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: A %-4i: trying to leave\n",shared->poradoveCislo,i);

    shared->poradoveCislo++;
    shared->adultEnded++;
    fprintf(zapis,"%-10i: A %-4i: leave\n",shared->poradoveCislo,i);

    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: A %-4i: waiting : %-2i: %-2i\n",shared->poradoveCislo,i,shared->AdultCount,shared->ChildCount);
    shared->adultEnded++;
    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: A %-4i: leave\n",shared->poradoveCislo,i);*/

    if (shared->adultEnded==A&&shared->childEnded==C)
        sem_post(&shared->konec);
    sem_wait(&shared->konec);
    shared->poradoveCislo++;
    fprintf(zapis,"%-10i: A %-4i: finished\n",shared->poradoveCislo,i);
    sem_post(&shared->konec);
    exit(0);
}
 /**
 * zabije vsechny processy v poli
 * parametr pole s pidy
 */
void kill_all(pid_t pids[])
{
    // Projde v cyklu pole s PID, kdyĹž narazĂ­ na prvnĂ­ 0, byl jiĹž odeslĂĄn signĂĄl vĹĄem potomkĂšm
    for(int i = 0; pids[i] != 0; ++i)
        // OdeslĂĄnĂ­ signĂĄlu pro ukonĂ¨enĂ­ procesu
        if(kill(pids[i], SIGTERM) == -1)
            // Pokud se nepodaĂ¸ilo zaslat signĂĄl, zĂ¸ejmĂŹ buĂŻ doĹĄlo k chybĂŹ v procesu potomka, nebo byl nĂĄsilnĂŹ ukonĂ¨en
            fprintf(stderr, "ERROR: Process %ld was unable to send signal SIGTERM to process %ld - call of kill failed!\nTry to terminate process manualy...\n", (long) getpid(), (long) pids[i]);
}
