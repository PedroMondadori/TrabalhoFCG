Alunos:
Alexandre de Rosso Crestani 00312980
Airton Junior               00275852
Pedro Lago Mondadori        00301506

Este é o nosso trabalho final da cadeira de Fundamentos de Computação Gráfica, ministrada pelo professor Eduardo Gastal.

Desenvolvemos um jogo de tiro simples, contendo:
    - um jogador, controlado pelo usuário;
    - um monstro, controlado pelo computador;
    - árvores, representando obstáculos;
    - e uma arena, contendo o chão e 4 paredes;
    - uma interface que mostra
        - os pontos de vida do jogador (de um total de 100);
        - Score (quantos monstros ja matou);
        - e a velocidade atual do monstro (dificuldade);

Funcionamento:
    - o jogador possui 100 de vida;
    - o monstro, quando enconsta no jogador, dá entre 20 e 30 de dano por segundo;
    - o monstro morre com um único tiro do jogador;
    - a cada vez que o monstro morre ele fica mais rápido (recebe entre 0 e 15 de velocidade adicional);
    - se o jogador não receber dano por 2 segundos, ele regenera 5 de vida por segundo;

Jogando:
    - para movimentar-se utilize as teclas W, A, S, D;
    - para mirar utilize o mouse e para atirar clique com o botão esquerdo do mouse;
    - é possível trocar a câmera para um 'modo sniper' apertando a tecla C (nesse modo o jogador é posicionado para o meio da arena e o monstro vai para a última localização do jogador sem ser a atual)
    - é possivel esconter a interface que mostra os status atuais do jogo apertando a tecla H

Executando o código:
    Para rodar a aplicação é necessário o uso da IDE Code::Blocks, que pode ser baixada em http://codeblocks.org/.
    Após isso, é necessário mudar o 'build target' do compilador do Code::Blocks de 'Debug' para 'Debug (CBlocks 17.12 32-bit)'
    Compilar e executar

