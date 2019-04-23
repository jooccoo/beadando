#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct meresi_adat //struktura letrehozasa a meresi adatok tarolasahoz
{
    char helyszin[200];
    int max_sebesseg;
    char datum[15];
    char ido[15];
    char vetseg[30];
    char rendszam[15];
    int sebesseg;
    int sebesseg_tullepes;
}MERESI_ADAT;

typedef struct fajl_adatok //struktura letrehozasa az osszesito adatokhoz
{
    char nev[200];
    int kihagasok_db;
}FAJL_ADATOK;

typedef struct kihagas_stat //struktura letrehozasa az osszesito adatokhoz
{
    char nev[100];
    int db;
}KIHAGAS_STAT;

void kihagasok_feltoltes(KIHAGAS_STAT* k) //kihagas struktura feltoltes az adatokkal
{
    int i=0;
    int j=0;
    while(i<7)
    {
        k[i].db=j;
        i++;
    }
    strcpy(k[0].nev,"gyorshajtas");
    strcpy(k[1].nev,"vasuti_atjaro");
    strcpy(k[2].nev,"piros_lampa");
    strcpy(k[3].nev,"leallosav");
    strcpy(k[4].nev,"behajtasi_tilalom");
    strcpy(k[5].nev,"kotelezo_haladas");
    strcpy(k[6].nev,"korlatozott_zona");
}

int hasonlit_megoszlas(const void* x1, const void* x2) // qsorthoz függvény, eloszlas rendezés csökkenőbe + név szerint
{
	FAJL_ADATOK* a = (FAJL_ADATOK*) x1;
	FAJL_ADATOK* b = (FAJL_ADATOK*) x2;
	if(a->kihagasok_db != b->kihagasok_db)
        return (b->kihagasok_db - a->kihagasok_db);
    else
        return strcmp(a->nev, b->nev);
}

int hasonlit_gyorshajto(const void* x1, const void* x2) // qsorthoz függvény, gyorshajtás rendezés csökkenőbe + név szerint
{
	MERESI_ADAT* a = (MERESI_ADAT*) x1;
	MERESI_ADAT* b = (MERESI_ADAT*) x2;
	if((a->sebesseg - a->max_sebesseg) != (b->sebesseg - b->max_sebesseg))//novekvobe rendez, mert akkor cserel, ha az elobbi nagyobb
		return (b->sebesseg - b->max_sebesseg) - (a->sebesseg - a->max_sebesseg);
    else
        return strcmp(a->rendszam,b->rendszam);
}

int hasonlit_rendszam(const void* x1, const void* x2) // qsorthoz függvény, rendszam szerint rendezés
{
	MERESI_ADAT* a = (MERESI_ADAT*) x1;
	MERESI_ADAT* b = (MERESI_ADAT*) x2;
	return strcmp(a->rendszam,b->rendszam);
}

void visszaesok_fix(MERESI_ADAT* p, int meresek_szama, FILE* ment) //visszaeso kihagok rendezese és kiírása
{
  qsort(p, meresek_szama, sizeof(MERESI_ADAT), hasonlit_rendszam); //rendszamok rendezese
  int i, db_rendszam=0;
  for(i = 0; i < meresek_szama-1; i++)
      if(strstr(p[i].rendszam, p[i+1].rendszam) != 0)
        {
            if(db_rendszam==0)
                {
                    fprintf(ment, "Visszatero szabalysertok:\n");
                    db_rendszam++;
                    if(strstr(p[i].rendszam, p[i+2].rendszam) != 0)
                        continue;
                    else
                    {
                        fprintf(ment, "%s\n",p[i].rendszam);
                        db_rendszam++;
                    }
                }
                else
                {
                    if(strcmp(p[i+1].rendszam, p[i+2].rendszam) != 0)
                    {
                    fprintf(ment,"%s\n",p[i].rendszam);
                    db_rendszam++;
                    }
                }
        }
        if(db_rendszam == 0)
            fprintf(ment, "Nincs visszatero szabalyserto.\n");
}

void hely_megoszlas(FAJL_ADATOK* v, int fajlok_szama, int osszes_meres_db, FILE* ment) //mérési hely szerinti eloszlas számolás és listázás
{
    int i;
    float szazalek;

    fprintf(ment, "Megoszlas ellenorzo pontok szerint:\n");
    for(i = 0; i < fajlok_szama; i++)
    {
        szazalek = ((double)v[i].kihagasok_db / (double)osszes_meres_db) *100;
        fprintf(ment, "%.2f%% - %s\n", szazalek , v[i].nev);
    }
    fprintf(ment,"\n");
}

void kihagas_megoszlas(KIHAGAS_STAT* w, int osszes_meres_db, FILE* ment) //kihagas szerinti eloszlas számolás és listázás
{
    fprintf(ment, "Megoszlas szabalysertes tipusa szerint:\n");
    int i=0;
    double szazalek;
    while(i<7)
    {
        szazalek = ((double)w[i].db / (double)osszes_meres_db) *100;
        fprintf(ment, "%.2f%% - %s\n", szazalek , w[i].nev);
        i++;
    }
    fprintf(ment,"\n");
}

void gyorshajtok(MERESI_ADAT* p, int gyorshajtok_db, FILE* ment) // Top XX gyorshajtok kiírása, XX<=10
{
    int j = 0;
    if(gyorshajtok_db > 10)
        fprintf(ment, "10 legnagyobb sebessegtullepes:\n");
    else
        fprintf(ment, "%d legnagyobb sebessegtullepes:\n",gyorshajtok_db);
    while( j < gyorshajtok_db)
    {
        fprintf(ment, "%s %s %s %s %d %d\n", p[j].helyszin, p[j].datum, p[j].ido, p[j].rendszam, p[j].max_sebesseg, p[j].sebesseg);
        j++;
        if(j==10)
            break;
    }
    fprintf(ment,"\n");
}

void kihagas_szamolas(KIHAGAS_STAT* p, char *kihagas_tip) // kihagasok osszesítése
{
    if(strstr(kihagas_tip,"vasuti_atjaro") > 0) //az adott vetsegek szamolasa
        p[1].db++;
    if(strstr(kihagas_tip,"piros_lampa") > 0)
        p[2].db++;
    if(strstr(kihagas_tip,"leallosav") > 0)
        p[3].db++;
    if(strstr(kihagas_tip,"behajtasi_tilalom") > 0)
        p[4].db++;
    if(strstr(kihagas_tip,"kotelezo_haladas") > 0)
        p[5].db++;
    if(strstr(kihagas_tip,"korlatozott_zona") > 0)
        p[6].db++;
}

int buntetes(MERESI_ADAT* p, int meresek_szama) //büntetések összesítése
{
    int i, tmp;
    long long int buntetes_osszege=0;
    for(i=0; i<meresek_szama; i++)
    {
        if(strstr(p[i].vetseg,"gyorshajtas")>0 && p[i].sebesseg != 0)
           {
                if(p[i].max_sebesseg <= 50)
                {
                    if(p[i].sebesseg_tullepes >= 15 && p[i].sebesseg_tullepes <= 25)
                        tmp=30000;
                    if(p[i].sebesseg_tullepes >= 26 && p[i].sebesseg_tullepes <= 35)
                        tmp=45000;
                    if(p[i].sebesseg_tullepes >= 36 && p[i].sebesseg_tullepes <= 45)
                        tmp=60000;
                    if(p[i].sebesseg_tullepes >= 46 && p[i].sebesseg_tullepes <= 55)
                        tmp=90000;
                    if(p[i].sebesseg_tullepes >= 56 && p[i].sebesseg_tullepes <= 65)
                        tmp=130000;
                    if(p[i].sebesseg_tullepes >= 66 && p[i].sebesseg_tullepes <= 75)
                        tmp=200000;
                    if(p[i].sebesseg_tullepes > 75)
                        tmp=300000;
                }
                if(p[i].max_sebesseg > 50 && p[i].max_sebesseg <= 100)
                {
                    if(p[i].sebesseg_tullepes >= 15 && p[i].sebesseg_tullepes <= 30)
                        tmp=30000;
                    if(p[i].sebesseg_tullepes >= 31 && p[i].sebesseg_tullepes <= 45)
                        tmp=45000;
                    if(p[i].sebesseg_tullepes >= 46 && p[i].sebesseg_tullepes <= 60)
                        tmp=60000;
                    if(p[i].sebesseg_tullepes >= 61 && p[i].sebesseg_tullepes <= 75)
                        tmp=90000;
                    if(p[i].sebesseg_tullepes >= 76 && p[i].sebesseg_tullepes <= 90)
                        tmp=130000;
                    if(p[i].sebesseg_tullepes >= 91 && p[i].sebesseg_tullepes <= 105)
                        tmp=200000;
                    if(p[i].sebesseg_tullepes > 105)
                        tmp=300000;
                }
                if(p[i].max_sebesseg > 100)
                {
                    if(p[i].sebesseg_tullepes >= 20 && p[i].sebesseg_tullepes <= 35)
                        tmp=30000;
                    if(p[i].sebesseg_tullepes >= 36 && p[i].sebesseg_tullepes <= 50)
                        tmp=45000;
                    if(p[i].sebesseg_tullepes >= 51 && p[i].sebesseg_tullepes <= 65)
                        tmp=60000;
                    if(p[i].sebesseg_tullepes >= 66 && p[i].sebesseg_tullepes <= 80)
                        tmp=90000;
                    if(p[i].sebesseg_tullepes >= 81 && p[i].sebesseg_tullepes <= 95)
                        tmp=130000;
                    if(p[i].sebesseg_tullepes >= 96 && p[i].sebesseg_tullepes <= 110)
                        tmp=200000;
                    if(p[i].sebesseg_tullepes > 110)
                        tmp=300000;
                }
                buntetes_osszege=buntetes_osszege+tmp;
           }
        if(strstr(p[i].vetseg,"vasuti_atjaro") >0)
            buntetes_osszege=buntetes_osszege+60000;
        if(strstr(p[i].vetseg,"piros_lampa") >0)
            buntetes_osszege=buntetes_osszege+50000;
        if(strstr(p[i].vetseg,"leallosav") >0)
            buntetes_osszege=buntetes_osszege+100000;
        if(strstr(p[i].vetseg,"behajtasi_tilalom") >0)
            buntetes_osszege=buntetes_osszege+30000;
        if(strstr(p[i].vetseg,"korlatozott_zona") >0)
            buntetes_osszege=buntetes_osszege+30000;
        if(strstr(p[i].vetseg,"kotelezo_haladas") >0)
            buntetes_osszege=buntetes_osszege+50000;
    }
    return buntetes_osszege;
}

void sorveg_karakter_csere(char* be)
{
    if(be[strlen ( be )-1] == '\n')
        be[strlen ( be )-1] = '\0'; // sorvégejel felülírása
}
int main(int argc, char *argv[]) // argumentum beolvasása
{
    MERESI_ADAT *t;
    FAJL_ADATOK *p;
    KIHAGAS_STAT k[8];
    int index=1, db=0, token_sebesseg, sor;
    FILE *fp;
    FILE *ment;
    char str[300];
    char *token;
    char token_hely[100];
    long long int bunti;

    ment = fopen("statisztika_2019_marcius.txt", "w");
    t = (MERESI_ADAT*)malloc(sizeof(MERESI_ADAT)); //memóriafoglalás
    p = (FAJL_ADATOK*)malloc(sizeof(FAJL_ADATOK));

    kihagasok_feltoltes(k); // kihagas struktura feltoltese

    while( index < argc) //az argumentumként megadott fájlok megnyitása egymás után
    {
        p = (FAJL_ADATOK*)realloc(p, (index) * sizeof(FAJL_ADATOK)); // memoria ujrafoglalas
        fp = fopen(argv[index], "r"); //fajlok megnyitasa
        sor=1; //sor számlálás fájlonként
        while( fgets (str, 300, fp) != NULL) //vegigfut az fajlokon soronkent
        {
            t = (MERESI_ADAT*)realloc(t, (db+1) * sizeof(MERESI_ADAT)); //memória újrafoglalás

            if(sor==1) // első sor, azaz a hely elmentése
            {
                token = strtok(str, "\n");
                strcpy(t[db].helyszin, token);
                strcpy(token_hely, token);
                strcpy(p[index-1].nev, token);
                db=db-1;
            }
            else if(sor==2) //második sor, azaz a sebességkorlát mentése
            {
                t[db].max_sebesseg = atoi(str); // string konvertálás egész számmá és elmentése az adott fajl 1.
                token_sebesseg = atoi(str); // string konvertálás egész számmá és globalis változóba mentés
                db=db-1;
            }
            else //tokenizálás
            {   sorveg_karakter_csere(str);
                if(strstr(str, "gyorshajtas") > 0) // tokenizálás HA a beolvasott sorban gyorshajtó van
                {
                    k[0].db++;
                    token = strtok(str, ";");
                    strcpy(t[db].datum, token);
                    token = strtok(NULL, ";");
                    strcpy(t[db].ido, token);
                    token = strtok(NULL, " ");
                    strcpy(t[db].vetseg, token);
                    token = strtok(NULL, ";");
                    t[db].sebesseg = atoi(token);
                    token = strtok(NULL, ";");
                    strcpy(t[db].rendszam, token);
                    t[db].sebesseg_tullepes = t[db].sebesseg - token_sebesseg;
                }
                else // tokenizálás, ha a nem gyorshajtorol van szó
                {
                    token = strtok(str, ";");
                    strcpy(t[db].datum, token);
                    token = strtok(NULL, ";");
                    strcpy(t[db].ido, token);
                    token = strtok(NULL, ";");
                    strcpy(t[db].vetseg, token);
                    kihagas_szamolas(k,token); //kihagasok osszesítese
                    t[db].sebesseg = 0;
                    t[db].sebesseg_tullepes = 0;
                    token = strtok(NULL, ";");
                    strcpy(t[db].rendszam, token);
                }

                if(db > 0 && sor !=1 ) //a hely es a sebességkorlát hozzárendelése az adott fajlok összes eleméhez
                {
                    strcpy(t[db].helyszin, token_hely);
                    t[db].max_sebesseg = token_sebesseg;
                }
            }
            sor++; //sorok számának a növelése
            db++; //mérések száma növelése
        }
        p[index-1].kihagasok_db=sor-3; //az adott fajlokban talalhato meresek szamanak mentese
        fclose(fp); // megnyitott fajlok bezarasa
        index++; //lepes a következõ fájlra
    }
    bunti=buntetes(t,db); // buntetés nagyságának átadása forintban
    fprintf(ment, "A honap soran %d birsag kerult kiszabasra, osszesen %lld forint ertekben.\n", db, bunti); // buntetesek szama és nagysága kiírűsa 1.feladat
    fprintf(ment,"\n");
    qsort(p, index-1, sizeof(FAJL_ADATOK), hasonlit_megoszlas); //quicksort rendés előfordulás szerint csökkenőbe
    hely_megoszlas(p,index-1,db, ment); //meres helye szerinti eloszlas 2.feladat
    kihagas_megoszlas(k,db,ment); // kihagasok összegzése, kiírása 3.feladat.
    qsort(t, db, sizeof(MERESI_ADAT), hasonlit_gyorshajto);
    gyorshajtok(t, k[0].db, ment); // gyorshajtok rendezese es listazasa 4.feladat
    visszaesok_fix(t, db, ment); //visszaeso szabalysertok 5.feladat
    fclose(ment);
    int i;
    free(t); //memoria felszabitas
    free(p);
    return 0;
}
