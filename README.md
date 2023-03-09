# MaquinaLavaRoupa

## Projeto de microcontrolador, simulando uma máquina de lavar louça

#
## Estados:
### A = Desligado;
### B = Aquecendo;
### C = Ciclo de Lavagem
### D = Saída de Água;
### E = Dispersão do Líquido Secante
### D = Escoamento do Líquido Secante

### Em cada estado a máquina deve mostrar o tempo que ela gasta para finalizar o mesmo e passar para o próximo estado.

#
## Comunicação

### A máquina faz comunicação com o broker, no qual:
### O usuário informa ao broker o tempo que ele quer para a duração do estado C, D e E
### A broker informa a máquina o tempo que ela deve funcionar nos determinados estados
