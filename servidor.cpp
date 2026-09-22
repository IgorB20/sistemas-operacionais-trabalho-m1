#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <regex>
#include <fstream>
#include <pthread.h>
#include <vector>
#include <semaphore.h>

using namespace std;

typedef struct
{
    int id;
    string nome;
} Registro;

pthread_mutex_t bancoMutex = PTHREAD_MUTEX_INITIALIZER;
vector<string> filaRequisicoes;
pthread_mutex_t filaMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t *sem_fila;

int getIdFromQuery(const string &query)
{
    regex selectPattern(
        R"(^select \* from users where id=([0-9]+)$)",
        regex_constants::icase);

    regex updatePattern(
        R"(^update users set name=[^']+ where id=([0-9]+)$)",
        regex_constants::icase);

    regex deletePattern(
        R"(^delete from users where id=([0-9]+)$)",
        regex_constants::icase);

    smatch match;

    if (regex_match(query, match, selectPattern))
    {
        return stoi(match[1].str());
    }

    if (regex_match(query, match, updatePattern))
    {
        return stoi(match[1].str());
    }

    if (regex_match(query, match, deletePattern))
    {
        return stoi(match[1].str());
    }

    return -1;
}

Registro buscarRegistro(int idBuscado)
{
    ifstream arquivo("banco.txt");

    if (!arquivo.is_open())
    {
        throw "Erro na conexão com o banco";
    }

    int id;
    string nome;
    Registro resultado;

    while (arquivo >> id)
    {
        arquivo.ignore();
        getline(arquivo, nome);

        if (id == idBuscado)
        {
            resultado.id = id;
            resultado.nome = nome;
            return resultado;
        }
    }

    throw "Registro com não encontrado";
}

void *abrirMemoriaRequisicao(const char *nome, size_t tamanho)
{
    int shm_fd = shm_open(nome, O_RDONLY, 0666);

    if (shm_fd == -1)
    {
        perror("Erro ao abrir memoria de requisicao");
        return nullptr;
    }

    void *ptr = mmap(
        nullptr,
        tamanho,
        PROT_READ,
        MAP_SHARED,
        shm_fd,
        0);

    close(shm_fd);

    if (ptr == MAP_FAILED)
    {
        perror("Erro ao mapear memoria de requisicao");
        return nullptr;
    }

    return ptr;
}

void *abrirMemoriaResposta(const char *nome, size_t tamanho)
{
    int shm_fd = shm_open(nome, O_CREAT | O_RDWR, 0666);

    if (shm_fd == -1)
    {
        perror("Erro ao abrir memoria de resposta");
        return nullptr;
    }

    ftruncate(shm_fd, tamanho);

    void *ptr = mmap(
        nullptr,
        tamanho,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        shm_fd,
        0);

    close(shm_fd);

    if (ptr == MAP_FAILED)
    {
        perror("Erro ao mapear memoria de resposta");
        return nullptr;
    }

    return ptr;
}

int getQueryType(const string &query)
{
    regex selectPattern(
        R"(^select\s+.+\s+from\s+.+$)",
        regex_constants::icase);

    regex updatePattern(
        R"(^update\s+users\s+set\s+name=[A-Za-z0-9 ]+\s+where\s+id=[0-9]+$)",
        regex_constants::icase);

    regex insertPattern(
        R"(^insert\s+into\s+users\s+[0-9]+,[A-Za-z ]+$)",
        regex_constants::icase);

    regex deletePattern(
        R"(^delete\s+from\s+users\s+where\s+id=[0-9]+$)",
        regex_constants::icase);

    if (regex_match(query, selectPattern))
    {
        return 1;
    }

    if (regex_match(query, updatePattern))
    {
        return 2;
    }

    if (regex_match(query, insertPattern))
    {
        return 3;
    }

    if (regex_match(query, deletePattern))
    {
        return 4;
    }

    throw runtime_error("Query invalida");
}

bool atualizarNome(int idBuscado, const string &novoNome)
{

    pthread_mutex_lock(&bancoMutex);
    ifstream arquivo("banco.txt");

    if (!arquivo.is_open())
    {
        return false;
    }

    vector<Registro> registros;

    int id;
    string nome;
    bool encontrado = false;

    while (arquivo >> id)
    {
        arquivo.ignore();
        getline(arquivo, nome);

        Registro r;

        r.id = id;

        if (id == idBuscado)
        {
            r.nome = novoNome;
            encontrado = true;
        }
        else
        {
            r.nome = nome;
        }

        registros.push_back(r);
    }

    arquivo.close();

    if (!encontrado)
    {
        return false;
    }

    ofstream arquivoSaida("banco.txt");

    for (const Registro &r : registros)
    {
        arquivoSaida
            << r.id
            << " "
            << r.nome
            << endl;
    }

    arquivoSaida.close();

    pthread_mutex_unlock(&bancoMutex);

    return true;
}

Registro select(string query)
{

    // SELECT QUERY VALIDATION
    int id = getIdFromQuery(query);

    if (id == -1)
    {
        cout << "Query invalida" << endl;
    }
    else
    {
        cout << "ID: " << id << endl;
    }
    // ----------------------

    // EXECUTION

    // busca sequencial dentro do txt pela linha com id informado
    // retornar not found caso não encontre
    Registro resultado = buscarRegistro(id);
    cout << "Registro encontrado: " << endl;
    cout << "ID: " << resultado.id << endl;
    cout << "Nome: " << resultado.nome << endl;
    return resultado;
}

string getNameFromUpdateQuery(const string &query)
{
    regex pattern(
        R"(^update\s+users\s+set\s+name=([A-Za-z]+)\s+where\s+id=[0-9]+$)",
        regex_constants::icase);

    smatch match;

    if (!regex_match(query, match, pattern))
    {
        throw runtime_error("Query update invalida");
    }

    return match[1].str();
}

Registro update(string query)
{
    // update users set name='Igor Benedet' where id=1
    int id = getIdFromQuery(query);

    if (id == -1)
    {
        cout << "Query invalida" << endl;
    }
    else
    {
        cout << "ID: " << id << endl;
    }

    string novoNome = getNameFromUpdateQuery(query);

    // editar novo nome
    if (atualizarNome(id, novoNome))
    {
        cout << "Usuario atualizado" << endl;

        Registro resultado = {id, novoNome};
        return resultado;
    }
    else
    {
        throw "Usuario nao encontrado";
    }
}

Registro getInsertData(const string &query)
{
    regex pattern(
        R"(^insert\s+into\s+users\s+([0-9]+),([A-Za-z ]+)$)",
        regex_constants::icase);

    smatch match;

    if (!regex_match(query, match, pattern))
    {
        throw runtime_error("Query insert invalida");
    }

    Registro data;

    data.id = stoi(match[1].str());
    data.nome = match[2].str();

    return data;
}

bool inserirRegistro(Registro registro)
{
    pthread_mutex_lock(&bancoMutex);

    ofstream arquivo("banco.txt", ios::app);

    if (!arquivo.is_open())
    {
        pthread_mutex_unlock(&bancoMutex);
        return false;
    }

    arquivo << registro.id << " " << registro.nome << endl;

    arquivo.close();

    pthread_mutex_unlock(&bancoMutex);

    return true;
}

Registro insert(string query)
{

    Registro registro = getInsertData(query);

    if (inserirRegistro(registro))
    {
        cout << "Registro inserido com sucesso" << endl;
        return registro;
    }
    else
    {
        throw "Erro ao inserir registro";
    }
}

bool excluirRegistro(int idBuscado)
{
    pthread_mutex_lock(&bancoMutex);

    ifstream arquivo("banco.txt");

    if (!arquivo.is_open())
    {
        pthread_mutex_unlock(&bancoMutex);
        return false;
    }

    vector<Registro> registros;

    int id;
    string nome;
    bool encontrado = false;

    while (arquivo >> id)
    {
        arquivo.ignore();
        getline(arquivo, nome);

        if (id == idBuscado)
        {
            encontrado = true;
            continue;
        }

        Registro r;
        r.id = id;
        r.nome = nome;

        registros.push_back(r);
    }

    arquivo.close();

    if (!encontrado)
    {
        pthread_mutex_unlock(&bancoMutex);
        return false;
    }

    ofstream arquivoSaida("banco.txt");

    if (!arquivoSaida.is_open())
    {
        pthread_mutex_unlock(&bancoMutex);
        return false;
    }

    for (const Registro &r : registros)
    {
        arquivoSaida
            << r.id
            << " "
            << r.nome
            << endl;
    }

    arquivoSaida.close();

    pthread_mutex_unlock(&bancoMutex);

    return true;
}

Registro deleteQuery(string query)
{
    int id = getIdFromQuery(query);

    if (id == -1)
    {
        cout << "Query invalida" << endl;
    }
    else
    {
        cout << "ID: " << id << endl;
    }

    if (excluirRegistro(id))
    {
        cout << "Registro excluido com sucesso" << endl;
        Registro registroDeletado;
        registroDeletado.id = id;
        registroDeletado.nome = "deletado";
        return registroDeletado;
    }
    else
    {
        throw "Registro nao encontrado";
    }
}

void *processarRequisicao(void *args)
{
    const char *memoria_resposta = "/memoria_resposta";
    string *input = static_cast<string *>(args);
    const size_t tamanho = 1024;

    sem_t *sem_resposta = sem_open(
        "/sem_resposta",
        O_CREAT,
        0666,
        0);

    string *query = input;

    // VALIDADOR
    //  QUAL TIPO DE QUERY (SELECT, UPDATE, INSERT, DELETE)
    int type = getQueryType(*query);

    Registro resultado;
    switch (type)
    {
    case 1:
        resultado = select(*query);
        break;
    case 2:
        resultado = update(*query);
        break;
    case 3:
        resultado = insert(*query);
        break;
    case 4:
        resultado = deleteQuery(*query);
        break;
    }

    // RESPOSTA

    void *ptr_resposta =
        abrirMemoriaResposta(memoria_resposta, tamanho);

    /*
     if (ptr_resposta == nullptr) {
            munmap(ptr_requisicao, tamanho);
            return 1;
    }*/

    string resposta =
        to_string(resultado.id) + " " + resultado.nome;

    // escrevendo resposta
    strcpy(
        static_cast<char *>(ptr_resposta),
        resposta.c_str());

    sem_post(sem_resposta);

    // Liberar recursos do processo B
    munmap(ptr_resposta, tamanho);

    delete query;

    return nullptr;
}

void *worker(void *arg)
{
    while (true)
    {
        sem_wait(sem_fila);

        pthread_mutex_lock(&filaMutex);

        string query = filaRequisicoes.front();
        filaRequisicoes.erase(filaRequisicoes.begin());

        pthread_mutex_unlock(&filaMutex);

        string *queryThread = new string(query);

        processarRequisicao(queryThread);
    }

    return nullptr;
}

int main()
{
    const char *memoria_requisicao = "/memoria_requisicao";

    const size_t tamanho = 1024;

    sem_t *sem_requisicao = sem_open(
        "/sem_requisicao",
        O_CREAT,
        0666,
        0);

    sem_fila = sem_open(
        "/sem_fila",
        O_CREAT,
        0666,
        0);

    pthread_t t1;
    pthread_t t2;

    pthread_create(&t1, nullptr, worker, nullptr);
    pthread_create(&t2, nullptr, worker, nullptr);

    while (true)
    {
        sem_wait(sem_requisicao);

        cout << "Nova requisicao recebida" << endl;

        void *ptr_requisicao =
            abrirMemoriaRequisicao(
                memoria_requisicao,
                tamanho);

        string command =
            static_cast<char *>(ptr_requisicao);

        pthread_mutex_lock(&filaMutex);

        filaRequisicoes.push_back(command);

        pthread_mutex_unlock(&filaMutex);

        sem_post(sem_fila);

        munmap(ptr_requisicao, tamanho);
    }

    return 0;
}
