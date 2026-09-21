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
```

## Requisitos

É necessário ter instalado:

- `g++`
- `make`
- suporte a POSIX Threads (`pthread`)

## Compilação

Para compilar o cliente e o servidor:

```bash
make
```

Isso irá gerar os executáveis:

```text
cliente
servidor
```

## Executando o servidor

Abra um terminal na pasta do projeto e execute:

```bash
make run-servidor
```

O servidor ficará aguardando requisições enviadas pelos clientes.

## Executando o cliente

Abra outro terminal na mesma pasta do projeto e execute:

```bash
make run-cliente
```

O cliente permitirá o envio de comandos para o servidor.

## Exemplo de execução

Primeiro terminal:

```bash
make run-servidor
```

Segundo terminal:

```bash
make run-cliente
```

No cliente, envie uma query como:

```text
select * from users where id=1
```

## Queries suportadas

### SELECT

Busca um usuário pelo ID.

Formato:

```text
select * from users where id=ID
```

Exemplo:

```text
select * from users where id=1
```

### UPDATE

Atualiza o nome de um usuário existente.

Formato:

```text
update users set name=NOVO_NOME where id=ID
```

Exemplo:

```text
update users set name=Igor where id=1
```

### INSERT

Insere um novo usuário no banco.

Formato:

```text
insert into users ID,NOME
```

Exemplo:

```text
insert into users 10,Novo Usuario
```

### DELETE

Remove um usuário pelo ID.

Formato:

```text
delete from users where id=ID
```

Exemplo:

```text
delete from users where id=1
```

## Banco de dados

O arquivo `banco.txt` simula o banco de dados.

Cada linha segue o formato:

```text
ID NOME
```

Exemplo:

```text
1 Lucas
2 Igor
3 Matheus
```

## Limpeza

Para remover os executáveis gerados:

```bash
make clean
```

## Comandos disponíveis

```bash
make
make servidor
make cliente
make run-servidor
make run-cliente
make clean
```