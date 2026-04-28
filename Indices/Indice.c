#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _Endereco Endereco;
typedef struct _Indice Indice;

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

struct _Indice
{
    char cep[8];
    long posicao;
};

static int comparaIndice(const void *a, const void *b)
{
    const Indice *ia = (const Indice *)a;
    const Indice *ib = (const Indice *)b;
    return strncmp(ia->cep, ib->cep, 8);
}

static void imprimeEndereco(const Endereco *e)
{
    printf("%.72s\n", e->logradouro);
    printf("%.72s\n", e->bairro);
    printf("%.72s\n", e->cidade);
    printf("%.72s\n", e->uf);
    printf("%.2s\n", e->sigla);
    printf("%.8s\n", e->cep);
}

int main(int argc, char **argv)
{
    FILE *arquivoDados;
    FILE *arquivoIndice;
    Endereco endereco;
    Indice *indice;
    char chaveCep[9];
    long quantidade;
    long i;
    long inicio;
    long fim;
    long meio;
    int resultado;
    int encontrado;

    if (argc != 2)
    {
        fprintf(stderr, "USO: %s [CEP]\n", argv[0]);
        return 1;
    }

    memset(chaveCep, 0, sizeof(chaveCep));
    strncpy(chaveCep, argv[1], sizeof(chaveCep) - 1);

    arquivoDados = fopen("cep_ordenado.dat", "rb");
    if (arquivoDados == NULL)
    {
        perror("Erro ao abrir arquivo de dados");
        return 1;
    }

    fseek(arquivoDados, 0, SEEK_END);
    quantidade = ftell(arquivoDados) / (long)sizeof(Endereco);
    rewind(arquivoDados);

    indice = (Indice *)malloc((size_t)quantidade * sizeof(Indice));
    if (indice == NULL)
    {
        fprintf(stderr, "Erro de memoria\n");
        fclose(arquivoDados);
        return 1;
    }

    for (i = 0; i < quantidade; i++)
    {
        long posicaoAtual = ftell(arquivoDados);
        if (fread(&endereco, sizeof(Endereco), 1, arquivoDados) != 1)
        {
            fprintf(stderr, "Erro ao ler registro %ld\n", i);
            free(indice);
            fclose(arquivoDados);
            return 1;
        }

        memcpy(indice[i].cep, endereco.cep, sizeof(indice[i].cep));
        indice[i].posicao = posicaoAtual;
    }

    qsort(indice, (size_t)quantidade, sizeof(Indice), comparaIndice);

    arquivoIndice = fopen("indice_ordenado.dat", "wb");
    if (arquivoIndice == NULL)
    {
        perror("Erro ao criar arquivo de indice");
        free(indice);
        fclose(arquivoDados);
        return 1;
    }

    if (fwrite(indice, sizeof(Indice), (size_t)quantidade, arquivoIndice) != (size_t)quantidade)
    {
        fprintf(stderr, "Erro ao gravar arquivo de indice\n");
        free(indice);
        fclose(arquivoIndice);
        fclose(arquivoDados);
        return 1;
    }

    fclose(arquivoIndice);

    inicio = 0;
    fim = quantidade - 1;
    encontrado = 0;

    while (inicio <= fim)
    {
        meio = (inicio + fim) / 2;
        resultado = strncmp(chaveCep, indice[meio].cep, 8);

        if (resultado == 0)
        {
            encontrado = 1;
            break;
        }
        else if (resultado < 0)
        {
            fim = meio - 1;
        }
        else
        {
            inicio = meio + 1;
        }
    }

    if (!encontrado)
    {
        printf("CEP nao encontrado: %s\n", chaveCep);
        free(indice);
        fclose(arquivoDados);
        return 0;
    }

    fseek(arquivoDados, indice[meio].posicao, SEEK_SET);
    if (fread(&endereco, sizeof(Endereco), 1, arquivoDados) != 1)
    {
        fprintf(stderr, "Erro ao recuperar registro original\n");
        free(indice);
        fclose(arquivoDados);
        return 1;
    }

    printf("Indice encontrado na posicao do arquivo: %ld\n", indice[meio].posicao);
    imprimeEndereco(&endereco);

    free(indice);
    fclose(arquivoDados);
    return 0;
}
