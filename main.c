// @autor: Nikola Djuza RA6-2012
// januar 2015.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SLOG_SLOBODAN 0
#define SLOG_AKTUELAN 1
#define LOGICKI_OBRISAN 2
#define _FB 3
#define _BRB 7
#define _K 1

char activeFile[20];
char current[20];

FILE *f;
FILE *pomocna;

typedef struct {
    int dan;
    int mesec;
    int godina;
    int sati;
    int minuti;
} Vreme;

typedef struct {
    int evidBroj;
    char regBroj[11];
    Vreme v;
    char oznRamp[5];
    int iznos;
    int status;
} Prolazak;

typedef struct {
    Prolazak slogovi[_FB];
} Baket;

// lista funkcija
int transformacijaID(int evidBroj);
void formirajDatoteku(char *imeDatoteke);
int upisiBaket(FILE *file, unsigned int adresa, Baket *bak);
void ucitajBaket(FILE *file, unsigned int adresa, Baket* bak);
void otvoriDatoteku(char *imeDatoteke);
int prolazakPostoji(FILE *file, int key, int *indBaketa, int *indLokacije);
void dodajProlazak();
void formirajAktivnuDatoteku();



int transformacijaID(int evidBroj) {
    return evidBroj%_BRB + 1;
}

void formirajDatoteku(char *imeDatoteke) {
    int i, j;

    fclose(f);
    if((f = fopen(imeDatoteke, "wb+")) == NULL) {
        printf("Datoteka %s nije napravljena!\n", imeDatoteke);
        return;
    }

    Baket bak;

    for(i = 0; i < _FB; i++) {
        bak.slogovi[i].evidBroj = -1;
        bak.slogovi[i].iznos = -1;

        for(j = 0; j < 4; j++)
            bak.slogovi[i].oznRamp[j] = ' ';
        bak.slogovi[i].oznRamp[4] = '\0';
        for(j = 0; j < 10; j++)
            bak.slogovi[i].regBroj[j] = ' ';
        bak.slogovi[i].regBroj[10] = '\0';

        bak.slogovi[i].status = SLOG_SLOBODAN;

        bak.slogovi[i].v.dan = 0;
        bak.slogovi[i].v.godina = 0;
        bak.slogovi[i].v.mesec = 0;
        bak.slogovi[i].v.minuti = 0;
        bak.slogovi[i].v.sati = 0;
    }

    for(i = 1; i <= _BRB; i++) {
        upisiBaket(f, i, &bak);
    }

    printf("Datoteka %s uspesno napravljena!\n", imeDatoteke);
    strcpy(activeFile, imeDatoteke);
}

int upisiBaket(FILE *file, unsigned int adresa, Baket *bak) {
	int ind;

	if(adresa < 1 || adresa > _BRB) {
		printf("Greska kod adrese baketa\n");
		return 0;
	}

	ind = fseek(file, (adresa-1)*sizeof(Baket), SEEK_SET);
	if(ind != 0) {
		printf("Greska pri seekovanju datoteke '%s'\n", activeFile);
		return 0;
	}

	ind = fwrite(bak, sizeof(Baket), 1, file);
	if(ind != 1) {
		printf("Greska pri upisu baketa u datoteku '%s'\n", activeFile);
		return 0;
	}

	return 1;
}

void ucitajBaket(FILE *file, unsigned int adresa, Baket *bak) {
	int ind;

	if(adresa < 1 || adresa > _BRB) {
		printf("Greska kod adrese baketa\n");
		return;
	}

	ind = fseek(file, (adresa-1)*sizeof(Baket), SEEK_SET);
	if(ind != 0) {
		printf("Greska pri seekovanju datoteke '%s'\n", activeFile);
		return;
	}

	ind = fread(bak, sizeof(Baket), 1, file);
	if(ind != 1) {
		printf("Greska pri citanju baketa iz datoteke '%s'\n", activeFile);
	} else {
        //printf("OK!\n");
	}

}

void otvoriDatoteku(char *imeDatoteke) {
    fclose(f);
    if((f = fopen(imeDatoteke, "rb+")) == NULL) {
        printf("\nDatoteka %s ne postoji!\n", imeDatoteke);
        return;
    }

    printf("\nDatoteka %s je uspesno otvorena!\n", imeDatoteke);
    strcpy(activeFile, imeDatoteke);
}

int prolazakPostoji(FILE *file, int key, int *indBaketa, int *indLokacije) {
    Baket baket;
    int maticniBaket, trenutniBaket;
    int j;

	maticniBaket = trenutniBaket = transformacijaID(key);

	do {
		ucitajBaket(file, trenutniBaket, &baket);

		for(j=0; j<_FB; j++) {   // faktor
			if(baket.slogovi[j].status == SLOG_AKTUELAN && baket.slogovi[j].evidBroj == key) {
                    // koristi se za promenu vremena i logicko brisanje
                    *indBaketa = trenutniBaket;
                    *indLokacije = j;
                    return 1;   // slog postoji i zauzet je
               }
		}

        // pomera se za fiksni korak k
		trenutniBaket += _K;

        // ako se prekoraci broj baketa
		if(trenutniBaket > _BRB)
			trenutniBaket = trenutniBaket % _BRB;

	} while(maticniBaket != trenutniBaket);

    return 0;
}

int prolazakPostojiUPomocnoj(int key) {
    fseek(pomocna, 0L, SEEK_END);
    int size = ftell(pomocna);
    //printf("\nVelicina pomocne datoteke %d", size);

    Prolazak prolazak;
    int adresa;

    for(adresa = 0; adresa < size/sizeof(Prolazak); adresa++) {
        if(fseek(pomocna, sizeof(Prolazak)*adresa, SEEK_SET) != 0) {
            printf("\nGreska prilikom seekovanja u pomocnoj datoteci.\n");
        }

        if(fread(&prolazak, sizeof(Prolazak), 1, pomocna) != 1) {
            printf("\nGreska prilikom read u pomocnoj datoteci.\n");
            return 0;
        }

        if(prolazak.evidBroj == key) {
            return 1;
        }
    }

    return 0;
}

void dodajProlazak() {
    Prolazak prolazak;

    bool ok = true;
    int indBaketa, indLokacije;

    if(f == NULL) {
        printf("\nNe postoji aktivna datoteka!\n");
    }

    // evidencioni broj
    do {
        ok = true;
        printf("\nEvidencioni broj (8 cifara): ");
        fflush(stdin);
        scanf("%d", &prolazak.evidBroj);

        // provera da li slog postoji u aktivoj datoteci
        if(prolazakPostoji(f, prolazak.evidBroj, &indBaketa, &indLokacije) == 1) {
            printf("\nSlog sa '%d' evidencionim brojem vec postoji na Slogu(%d, %d)\n", prolazak.evidBroj
                   , indBaketa, indLokacije + 1);
            ok = false;
        }

        if(prolazakPostojiUPomocnoj(prolazak.evidBroj) == 1) {
            printf("\nSlog sa '%d' evidencionim brojem vec postoji u pomocnoj datoteci\n", prolazak.evidBroj);
            ok = false;
        }

        // provera duzine unosa
        if((prolazak.evidBroj > 99999999) || (prolazak.evidBroj < 10000000)) {
            printf("\nEvidencioni broj se sastoji iz osam(8) cifara.");
            ok = false;
        }
    } while(!ok);

    // registarska oznaka vozila
    do {
        ok = true;
        printf("\nRegistarska oznaka vozila (10 karaktera): ");
        fflush(stdin);
        gets(prolazak.regBroj);

        if(strlen(prolazak.regBroj) > 10) {
            printf("\nRegistarska oznaka vozila se sastoji iz deset(10) karaktera.");
            ok = false;
        }
    } while(!ok);

    printf("Datum:\n");
    printf("Dan: ");
    fflush(stdin);
    scanf("%d", &prolazak.v.dan);
    printf("Mesec: ");
    fflush(stdin);
    scanf("%d", &prolazak.v.mesec);
    printf("Godina: ");
    fflush(stdin);
    scanf("%d", &prolazak.v.godina);
    printf("Sati: ");
    fflush(stdin);
    scanf("%d", &prolazak.v.sati);
    printf("Minuti: ");
    fflush(stdin);
    scanf("%d", &prolazak.v.minuti);

    // oznaka naplatne rampe
    do {
        ok = true;
        printf("\nOznaka naplatne rampe (4 karaktera): ");
        fflush(stdin);
        gets(prolazak.oznRamp);

        if(strlen(prolazak.oznRamp) > 4 || strlen(prolazak.oznRamp) < 4) {
            printf("\nRegistarska oznaka vozila se sastoji iz cetiri(4) karaktera.");
            ok = false;
        }
    } while(!ok);

    // placeni iznos
    do {
        ok = true;
        printf("\nPlaceni iznos (do 100 000): ");
        fflush(stdin);
        scanf("%d", &prolazak.iznos);

        if(prolazak.iznos > 100000 || prolazak.iznos < 0) {
            printf("\nPlaceni iznos se nalazi izmedju 0 i 100 000");
            ok = false;
        }
    } while(!ok);

    prolazak.status = SLOG_AKTUELAN;

    // pozicioniranje na kraj datoteke radi upisa
    fseek(pomocna, 0L, SEEK_END);

    if((fwrite(&prolazak, sizeof(Prolazak), 1, pomocna)) != 1){
        printf("\nGreska prilikom punjenja pomocne datoteke!\n");
        return;
    }
    printf("\nSlog je uspesno unet u pomocnu datoteku!\n");
}

void formirajAktivnuDatoteku() {
    Baket baket;
    Prolazak prolazak;
    fseek(pomocna, 0L, SEEK_END);
    int size = ftell(pomocna);
    printf("\nVelicina pomocne datoteke %d\n", size);

    int adresa;
    int upisan = 0;
    int i;
    int maticniBaket, trenutniBaket;

    for(adresa = 0; adresa < size/sizeof(Prolazak); adresa++) {
        if(fseek(pomocna, sizeof(Prolazak)*adresa, SEEK_SET) != 0) {
            printf("Greska prilikom seekovanja u pomocnoj datoteci.\n");
        }

        if(fread(&prolazak, sizeof(Prolazak), 1, pomocna) != 1) {
            printf("Greska prilikom read u pomocnoj datoteci.\n");
            return;
        }
        // Generisanje adrese maticnog baketa
        maticniBaket = trenutniBaket = transformacijaID(prolazak.evidBroj);

        do
        {
            ucitajBaket(f, trenutniBaket, &baket);

            // prolazak kroz slogove baketa
            for(i = 0; i < _FB; i++)
            {
                // da li je lokacija sloga slobodna?
                if(baket.slogovi[i].status == SLOG_SLOBODAN)
                {
                    // ako jeste upisujemo slog na to mesto
                    baket.slogovi[i] = prolazak;
                    upisan = 1;
                    prikaziSlog(&baket.slogovi[i]);
                    break;
                }
            }

            if(!upisan)
            {
                // ukoliko trenutan baket nije slobodan, uvecavamo adresu baketa za fiksni korak k
                trenutniBaket += _K;

                // Ako smo presli B baketa, delimo adresu sa modulom B
                if(trenutniBaket > _BRB)
                    trenutniBaket = trenutniBaket % _BRB;
            }

        } while(!upisan && trenutniBaket != maticniBaket);
         // zaustavlja se ili ako naleti na slobodnu lokaciju ili ako dodje do maticnog baketa


        if(upisan != 0) {
            // pisemo baket nazad u datoteku
            printf("Trenutni baket = %d , maticni baket = %d", trenutniBaket, maticniBaket);
            if(upisiBaket(f, trenutniBaket, &baket) == 1) {
                printf("\nSlog uspesno upisan na adresu (%d,%d)\n", trenutniBaket, i+1);
                upisan = 0;
                // provera prekoracioca
                if(trenutniBaket != maticniBaket) {
                     printf("Slog je prekoracio iz maticnog baketa %d u %d\n", maticniBaket, trenutniBaket);
                }

            }
            else
                printf("\nSlog nije uspesno upisan.\n");

        }
        else {
            printf("\nGreska pri pisanju sloga (datoteka je puna)\n");
        }
    }

    // brisanje pomocne datoteke posle unosa
    if((pomocna = fopen("pomocna.bin", "wb+")) == NULL) {
        printf("\nNeuspesno brisanje pomocne datoteke posle formiranja aktivne datoteke!\n");
    }
}

void prikaziPomocnuDatoteku(){
    Prolazak prolazak;
    int i = 0;
    for(i = 0; i < _BRB*_FB; i++) {
        if(fseek(pomocna, sizeof(Prolazak)*i, SEEK_SET) != 0) {
            printf("Greska prilikom seekovanja u pomocnoj datoteci.\n");
        }

        if(fread(&prolazak, sizeof(Prolazak), 1, pomocna) != 1) {
            printf("\nGreska prilikom read u pomocnoj datoteci or EOF.\n");
            return;
        }

        prikaziSlog(&prolazak);
    }
}

void prikaziSlog(Prolazak *slog) {
    printf("\n\t==================================");
    printf("\n\tEvidencioni broj prolaska: %d", slog -> evidBroj);
    printf("\n\tRegistarski broj vozila: %s", slog -> regBroj);
    printf("\n\tVreme prolaska: %d.%d.%d %d:%d", slog -> v.dan, slog -> v.mesec, slog -> v.godina, slog -> v.sati, slog -> v.minuti);
    printf("\n\tOznaka rampe: %c%c%c%c", slog -> oznRamp[0], slog -> oznRamp[1], slog -> oznRamp[2], slog -> oznRamp[3]);
    printf("\n\tPlaceni iznos: %d", slog -> iznos);
    if(slog -> status == SLOG_SLOBODAN)
        printf("\n\tStatus sloga: SLOG SLOBODAN");

    if(slog -> status == SLOG_AKTUELAN)
        printf("\n\tStatus sloga: SLOG AKTUELAN");

    if(slog -> status == LOGICKI_OBRISAN)
        printf("\n\tStatus sloga: LOGICKI OBRISAN");

    printf("\n\t==================================\n");
}

void prikaziSveSlogove() {
    int i;
	int j;
	Baket baket;

	printf("\nPrikaz slogova datoteke (adresa baketa, redni broj sloga)\n");

	for(i = 1; i <= _BRB; i++)
	{
		ucitajBaket(f, i, &baket);

		for(j = 0; j < _FB; j++)
		{
			//if(baket.slogovi[j].status != SLOG_SLOBODAN)
			{
				printf("\n\nSlog (%d, %d)\n", i, j+1);

				prikaziSlog(&baket.slogovi[j]);
			}
		}
	}
}

void logickoBrisanje() {
    Baket baket;
    int evidBroj, indBaketa, indLokacije;

    printf("\nUnesite evidencioni broj prolaza kojeg zelite da obrisete: ");
    fflush(stdin);
    scanf("%d", &evidBroj);

    if(prolazakPostoji(f, evidBroj, &indBaketa, &indLokacije) == 0) {
        printf("\nProlaz sa evid. brojem: %d ne postoji.", evidBroj);
        return;
    }

    ucitajBaket(f, indBaketa, &baket);

    //resetSlog(&baket.slogovi[indLokacije]);
    baket.slogovi[indLokacije].status = LOGICKI_OBRISAN;

    if(upisiBaket(f, indBaketa, &baket) == 1) {
        printf("\nSlog (%d, %d) je logicki obrisan", indBaketa, indLokacije + 1);
    } else {
        printf("\nGreska prilikom logickom brisanja sloga (%d, %d)", indBaketa, indLokacije + 1);
    }
}

void resetSlog(Prolazak *slog) {
    int j;

    slog -> evidBroj = -1;
    slog -> iznos = -1;

    for(j = 0; j < 4; j++)
        slog -> oznRamp[j] = ' ';
    slog -> oznRamp[4] = '\0';
    for(j = 0; j < 10; j++)
        slog -> regBroj[j] = ' ';
    slog -> regBroj[10] = '\0';

    slog -> status = SLOG_SLOBODAN;

    slog -> v.dan = 0;
    slog -> v.godina = 0;
    slog -> v.mesec = 0;
    slog -> v.minuti = 0;
    slog -> v.sati = 0;
}

void promeniVreme() {
    Baket baket;
    Prolazak slog;
    int evidBroj, indBaketa, indLokacije;

    printf("\nUnesite evidencioni broj prolaza kojem zelite da promenite vreme: ");
    fflush(stdin);
    scanf("%d", &evidBroj);

    if(prolazakPostoji(f, evidBroj, &indBaketa, &indLokacije) == 0) {
        printf("\nProlaz sa evid. brojem: %d ne postoji.", evidBroj);
        return;
    }

    ucitajBaket(f, indBaketa, &baket);
    slog = baket.slogovi[indLokacije];
    printf("\nProlaz kojem zelite da promenite vreme: ");
    prikaziSlog(&slog);
    printf("\nUnesite novo vreme\n");

    printf("Datum:\n");
    printf("Dan: ");
    fflush(stdin);
    scanf("%d", &slog.v.dan);
    printf("Mesec: ");
    fflush(stdin);
    scanf("%d", &slog.v.mesec);
    printf("Godina: ");
    fflush(stdin);
    scanf("%d", &slog.v.godina);
    printf("Sati: ");
    fflush(stdin);
    scanf("%d", &slog.v.sati);
    printf("Minuti: ");
    fflush(stdin);
    scanf("%d", &slog.v.minuti);

    baket.slogovi[indLokacije] = slog;
    if(upisiBaket(f, indBaketa, &baket) == 1) {
        printf("\nSlogu (%d, %d) je uspesno promenjeno vreme\n", indBaketa, indLokacije + 1);
    } else {
        printf("\nGreska prilikom izmene vremena slogu (%d, %d)\n", indBaketa, indLokacije + 1);
    }
}

int main()
{
    int odg = 0;

    if((pomocna = fopen("pomocna.bin", "wb+")) == NULL) {
        printf("\nNeuspesno otvaranje pomocne datoteke!\n");
        return 0;
    }

    //printf("\nSize of Prolazak: %d\n", sizeof(Prolazak));
    printf("\n\t== Naplatna rampa kod mesta GRAD == \n");

    do {
        printf("\n\t============== MENI ===============\n");
        printf("\t1. Formiranje nove datoteka\n");
        printf("\t2. Izbor aktivne datoteka\n");
        printf("\t3. Prikaz naziva aktivne datoteka\n");
        printf("\t4. Upis novog sloga u pomocnu serijsku datoteku\n");
        printf("\t5. Formiranje aktivne datoteka citanjem iz pomocne\n");
        printf("\t6. Prikaz svih slogova\n");
        printf("\t7. Logicko brisanje aktuelnog sloga\n");
        printf("\t8. Izmena datuma i vremena sloga\n");
        printf("\t9. Ispis sadrzaja pomocne datoteke\n");
        printf("\t0. Izlaz iz programa\n");
        printf("\tUnesite Vas izbor: ");
        scanf("%d", &odg);

        switch(odg) {
        case 1:
            printf("\nUnesite ime datoteke: ");
            scanf("%s", activeFile);
            formirajDatoteku(activeFile);
            break;
        case 2:
            printf("\nUnesite ime datoteke koju zelite da otvorite: ");
            scanf("%s", current);
            otvoriDatoteku(current);
            break;
        case 3:
            if(f == NULL) {
                printf("\nNe postoji aktivna datoteka!\n");
            } else {
                printf("\nAktivna datoteka je %s\n", activeFile);
            }
            break;
        case 4:
            dodajProlazak();
            break;
        case 5:
            if(f == NULL) {
                printf("\nNe postoji aktivna datoteka!\n");
            } else {
                formirajAktivnuDatoteku();
            }
            break;
        case 6:
            if(f == NULL) {
                printf("\nNe postoji aktivna datoteka!\n");
            } else {
                prikaziSveSlogove();
            }
            break;
        case 7:
            if(f == NULL) {
                printf("\nNe postoji aktivna datoteka!\n");
            } else {
                logickoBrisanje();
            }
            break;
        case 8:
            if(f == NULL) {
                printf("\nNe postoji aktivna datoteka!\n");
            } else {
                promeniVreme();
            }
            break;
        case 9:
            prikaziPomocnuDatoteku();
            break;
        case 0:
            fclose(f);
            fclose(pomocna);
            exit(0);
            break;
        }
    } while(odg > 0 && odg < 10);

    return 0;
}
