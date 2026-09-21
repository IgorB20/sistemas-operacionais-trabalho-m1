# Sistema Cliente-Servidor com Memória Compartilhada

Projeto desenvolvido em C++ para simular um sistema cliente-servidor utilizando:

- Memória compartilhada POSIX
- Semáforos
- Threads com `pthread`
- Mutex
- Arquivo `.txt` simulando um banco de dados

## Estrutura do projeto

```text
.
├── cliente.cpp
├── servidor.cpp
├── banco.txt
├── Makefile
└── README.md
Requisitos

É necessário ter instalado:

g++
make
suporte a POSIX Threads (pthread)
Compilação

Para compilar o cliente e o servidor:

make

Isso irá gerar os executáveis:

cliente
servidor
Executando o servidor

Abra um terminal na pasta do projeto e execute:

make run-servidor

O servidor ficará aguardando requisições enviadas pelos clientes.

Executando o cliente

Abra outro terminal na mesma pasta do projeto e execute:

make run-cliente

O cliente permitirá o envio de comandos para o servidor.

Exemplo

Primeiro terminal:

make run-servidor

Segundo terminal:

make run-cliente

No cliente, envie uma query como:

select * from users where id=1
Queries suportadas
SELECT
select * from users where id=1
UPDATE
update users set name=Igor where id=1
INSERT
insert into users 10,Novo Usuario
DELETE
delete from users where id=1
Banco de dados

O arquivo banco.txt simula o banco de dados.

Cada linha segue o formato:

ID NOME

Exemplo:

1 Lucas
2 Igor
3 Matheus
Limpeza

Para remover os executáveis gerados:

make clean
Comandos disponíveis
make
make servidor
make cliente
make run-servidor
make run-cliente
make clean