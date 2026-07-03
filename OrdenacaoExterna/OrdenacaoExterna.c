#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _Endereco Endereco;

struct _Endereco
{
    char logradouro[72];
    char bairro[72];
    char cidade[72];
    char uf[72];
    char sigla[2];
    char cep[8];
    char lixo[2];
};

int compara(const void *e1, const void *e2)
{
    return strncmp(((Endereco *)e1)->cep, ((Endereco *)e2)->cep, 8);
}

void intercala(FILE *a, FILE *b, FILE *saida)
{
    Endereco ea, eb;

    fread(&ea, sizeof(Endereco), 1, a);
    fread(&eb, sizeof(Endereco), 1, b);

    while (!feof(a) && !feof(b))
    {
        if (compara(&ea, &eb) <= 0)
        {
            fwrite(&ea, sizeof(Endereco), 1, saida);
            fread(&ea, sizeof(Endereco), 1, a);
        }
        else
        {
            fwrite(&eb, sizeof(Endereco), 1, saida);
            fread(&eb, sizeof(Endereco), 1, b);
        }
    }

    while (!feof(a))
    {
        fwrite(&ea, sizeof(Endereco), 1, saida);
        fread(&ea, sizeof(Endereco), 1, a);
    }

    while (!feof(b))
    {
        fwrite(&eb, sizeof(Endereco), 1, saida);
        fread(&eb, sizeof(Endereco), 1, b);
    }
}

int main(int argc, char **argv)
{
    FILE *entrada, *saida, *a, *b;
    Endereco *bloco;
    int k, nBlocos, i;
    long totalRegistros, registrosPorBloco, resto, inicioRegistro;
    char nomeArquivo[64], nomeSaida[64];
    int round;

    if (argc != 3)
    {
        return 1;
    }

    k = atoi(argv[2]);

    if (k < 2 || (k & (k - 1)) != 0)
    {
        return 1;
    }

    entrada = fopen(argv[1], "rb");
    if (!entrada)
    {
        return 1;
    }

    fseek(entrada, 0, SEEK_END);
    totalRegistros = ftell(entrada) / sizeof(Endereco);
    rewind(entrada);

    printf("Total de registros: %ld\n", totalRegistros);
    printf("K = %d\n", k);

    registrosPorBloco = totalRegistros / k;
    resto = totalRegistros % k;

    printf("\nDividindo e ordenando %d blocs\n", k);

    inicioRegistro = 0;
    for (i = 0; i < k; i++)
    {
        long qtBloco = registrosPorBloco + (i < resto ? 1 : 0);

        bloco = (Endereco *)malloc((size_t)qtBloco * sizeof(Endereco));
        if (!bloco)
        {
            fclose(entrada);
            return 1;
        }

        fseek(entrada, inicioRegistro * sizeof(Endereco), SEEK_SET);
        if (fread(bloco, sizeof(Endereco), (size_t)qtBloco, entrada) != (size_t)qtBloco)
        {
            free(bloco);
            fclose(entrada);
            return 1;
        }

        qsort(bloco, (size_t)qtBloco, sizeof(Endereco), compara);

        sprintf(nomeArquivo, "temp_0_%d.dat", i);
        saida = fopen(nomeArquivo, "wb");
        if (!saida)
        {
            free(bloco);
            fclose(entrada);
            return 1;
        }

        fwrite(bloco, sizeof(Endereco), (size_t)qtBloco, saida);
        fclose(saida);
        free(bloco);

        inicioRegistro += qtBloco;
        printf("Bloco %d: %ld registros\n", i, qtBloco);
    }

    fclose(entrada);

    nBlocos = k;
    round = 0;

    while (nBlocos > 1)
    {
        int novoNBlocos = 0;

        for (i = 0; i < nBlocos; i += 2)
        {
            sprintf(nomeArquivo, "temp_%d_%d.dat", round, i);
            a = fopen(nomeArquivo, "rb");
            if (!a)
            {
                return 1;
            }

            sprintf(nomeArquivo, "temp_%d_%d.dat", round, i + 1);
            b = fopen(nomeArquivo, "rb");
            if (!b)
            {
                fclose(a);
                return 1;
            }

            sprintf(nomeSaida, "temp_%d_%d.dat", round + 1, novoNBlocos);
            saida = fopen(nomeSaida, "wb");
            if (!saida)
            {
                fclose(a);
                fclose(b);
                return 1;
            }

            intercala(a, b, saida);

            fclose(a);
            fclose(b);
            fclose(saida);

            sprintf(nomeArquivo, "temp_%d_%d.dat", round, i);
            remove(nomeArquivo);
            sprintf(nomeArquivo, "temp_%d_%d.dat", round, i + 1);
            remove(nomeArquivo);

            novoNBlocos++;
        }

        nBlocos = novoNBlocos;
        round++;
    }

    sprintf(nomeArquivo, "temp_%d_0.dat", round);
    if (rename(nomeArquivo, "cep_ordenado.dat") != 0)
    {
        return 1;
    }

    printf("\nArquivo ordenado gerado: cep_ordenado.dat\n");
    return 0;
}
