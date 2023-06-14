# Controle-de-Acesso-Com-Arduino-e-Esp8266
Repositorio para o Projeto de Controle de Acesso


Esse repositorio contem codigos para um controlador de acesso desenvolvido pela TITANIWM.

O Controlador consite em um leitor RFID, um teclado matricial 4x4, um display LCD 16x2 conectado a um arduino uno. O arduino uno se conecta atraves de uma conexao serial via software com um nodemcu(esp8266) que faz requisicoes http há uma api.

A Api faz diversas tratativas quando ao cartão lido ou senha digitada no teclado, e então libera ou não acesso.

<iframe width="640" height="394" src="https://www.youtube.com/embed/nXhs9-V_dKY" title="Demonstração Controlador de Acesso" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share" allowfullscreen></iframe>
