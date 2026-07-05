#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CSVParser.h"

#define READ_BUF_SIZE 8192
#define QT_PAISES 13

static const char *PAISES[QT_PAISES] = {
    "Argentina", "Bolivia", "Brazil", "Chile",
    "Colombia", "Ecuador", "French Guiana", "Guyana",
    "Paraguay", "Peru", "Suriname", "Uruguay", "Venezuela"};

typedef struct _DadosCovid
{
    long totalCasos[QT_PAISES];
    long totalMortes[QT_PAISES];
    int cabecalhoLido;
    int idxLocation, idxCases, idxDeaths;
    long linhas;
} DadosCovid;

static int achaColuna(char **cols, int ncols, const char *nome)
{
    for (int i = 0; i < ncols; i++)
        if (strcmp(cols[i], nome) == 0)
            return i;

    return -1;
}

static int paraInt(const char *s)
{
    if (s == NULL || s[0] == '\0')
        return 0;

    return atoi(s);
}

void processa(char **cols, int ncols, void *userData)
{
    DadosCovid *d = (DadosCovid *)userData;
    if (!d->cabecalhoLido)
    {
        d->idxLocation = achaColuna(cols, ncols, "location");
        d->idxCases = achaColuna(cols, ncols, "total_cases");
        d->idxDeaths = achaColuna(cols, ncols, "total_deaths");
        d->cabecalhoLido = 1;
        return;
    }

    d->linhas++;

    if (d->idxLocation >= ncols || d->idxCases >= ncols || d->idxDeaths >= ncols)
        return;

    for (int i = 0; i < QT_PAISES; i++)
    {
        if (strcmp(cols[d->idxLocation], PAISES[i]) == 0)
        {
            int casos = paraInt(cols[d->idxCases]);
            int mortes = paraInt(cols[d->idxDeaths]);
            if (casos > d->totalCasos[i])
                d->totalCasos[i] = casos;
            if (mortes > d->totalMortes[i])
                d->totalMortes[i] = mortes;
            break;
        }
    }
}

int main(int argc, char **argv)
{
    const char *entrada = (argc >= 2) ? argv[1] : "owid-covid-data.csv";

    FILE *f = fopen(entrada, "rb");
    if (!f)
    {
        return 1;
    }

    DadosCovid d;
    memset(&d, 0, sizeof(d));
    CSVParser csv;
    CSVParser_init(&csv);
    int qt;
    char ultimo = '\n';
    char *buf = (char *)malloc(READ_BUF_SIZE);

    while ((qt = fread(buf, 1, READ_BUF_SIZE, f)) > 0)
    {
        CSVParser_processLines(&csv, buf, qt, processa, &d);
        ultimo = buf[qt - 1];
    }
    if (ultimo != '\n')
        CSVParser_processLines(&csv, "\n", 1, processa, &d);

    free(buf);
    fclose(f);

    long totalCasosGeral = 0, totalMortesGeral = 0;

    printf("\n%-22s %18s %18s\n", "Pais", "Total Casos", "Total Mortes");
    for (int i = 0; i < QT_PAISES; i++)
    {
        printf("%-22s %18ld %18ld\n",
               PAISES[i], d.totalCasos[i], d.totalMortes[i]);
        totalCasosGeral += d.totalCasos[i];
        totalMortesGeral += d.totalMortes[i];
    }
    printf("%-22s %18ld %18ld\n", "TOTAL AMERICA SUL", totalCasosGeral, totalMortesGeral);
    return 0;
}
