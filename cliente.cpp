#include <iostream>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <semaphore.h>

void *abrirMemoriaRequisicao(const char *nome, size_t tamanho)
{
    int shm_fd = shm_open(nome, O_CREAT | O_RDWR, 0666);

    if (shm_fd == -1)
    {
        perror("Erro ao abrir memoria de requisicao");
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

int main()
{
    const char *memoria_requisicao = "/memoria_requisicao";
    const char *memoria_resposta = "/memoria_resposta";
    const size_t tamanho = 1024;

    // SEMAFOROS
    sem_t *sem_requisicao = sem_open(
        "/sem_requisicao",
        O_CREAT,
        0666,
        0);

    sem_t *sem_resposta = sem_open(
        "/sem_resposta",
        O_CREAT,
        0666,
        0);

    void *ptr_requisicao =
        abrirMemoriaRequisicao(memoria_requisicao, tamanho);

    if (ptr_requisicao == nullptr)
    {
        return 1;
    }

    void *ptr_resposta =
        abrirMemoriaResposta(memoria_resposta, tamanho);

    if (ptr_resposta == nullptr)
    {
        munmap(ptr_requisicao, tamanho);
        return 1;
    }

    std::string mensagem;

    while (true)
    {
        std::cout << "Manda um comando ai: " << std::endl;
        std::getline(std::cin, mensagem);

        // Escrever dados na memória compartilhada
        sprintf((char *)ptr_requisicao, "%s", mensagem.c_str(), "\0");
        std::cout << "Mensagem escrita na memória compartilhada." << std::endl;
        sem_post(sem_requisicao);

        sem_wait(sem_resposta);

        // le resposta
        std::string resposta =
            static_cast<char *>(ptr_resposta);

        std::cout << "Resposta do servidor: "
                  << resposta
                  << std::endl;
    }

    // Fechar mapeamento básico (remover depois no processo finalizador)
    munmap(ptr_requisicao, tamanho);

    return 0;
}
