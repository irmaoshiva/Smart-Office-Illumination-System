# Smart-Office-Illumination-System
Real-Time Cooperative Decentralized Control of a Smart Office Illumination System


# DUVIDAS

->METER TUDO A LER DA EPROM, E CALCULAR OS K'S
-> meter index= my_addr! e meter isto na eprom, cuidado com o que metemos como variaveis de entrada
-> do consensus para controlador deve ir o d ou d_av?
-> o controlador deve ser atualizado com os novos valores sempre ou so no final do consensus?

# [calibração]
-> meter tudo com N generico e nao de 0 a 2;
-> meter uma flag para so chamar o start calibration uma vez!!


## CONSENSUS:
->iluminaçao a mais do que o sistema nao consegue dar! quero 5000 lux! ele calcula todas e nenhuma é fazivel, temos de decidir o que fazer nesse caso. programar isso;
-> condição do erro ( vai dar poucas iterações)

meter os inteiros em uint_8;

Pôr destructors nas classes (pelo menos perguntar ao prof se não é preciso);

[DIZER CASOS QUE ESTIVEMOS A TESTAR ]:
->CALIBRAÇÃO:
Dizer que metemos os 2 a correr ao mm tempo;
Dizer que metemos um de cada vez;
Dizer que metemos 1º o 1, e depois o 2, e vice-versa;
