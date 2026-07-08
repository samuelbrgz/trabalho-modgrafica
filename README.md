# Sistema Solar 3D - Modelagem Gráfica

Este projeto é uma aplicação gráfica 3D interativa desenvolvida em **C** utilizando a biblioteca **FreeGLUT/OpenGL**. Ele simula um sistema estelar navegável com planetas, órbitas, objetos animados e colisão dinâmica, desenvolvido como Trabalho Prático para a disciplina de Modelagem Gráfica da Faculdade de Engenharia da UFMT.

## 🚀 Funcionalidades
* **Navegação Livre (FPS):** Movimentação em 3 eixos e rotação de câmera suave controlada pelo mouse.
* **Modelagem Hierárquica:** Sistema planetário complexo contendo o Sistema Terra-Lua (com decalques nos continentes), anéis de Saturno e um foguete composto espacial pulsante.
* **Iluminação Dinâmica:** Fonte de luz central posicionada no Sol utilizando o modelo de iluminação de Phong (componentes Ambiente, Difusa e Especular).
* **Física e Colisão:** Cinemática básica aplicada à mecânica de pulo do observador e detecção contínua de colisão esférica contra o Sol.
* **HUD em Tempo Real:** Exibição 2D sobreposta na tela com feedback instantâneo de coordenadas, modos de projeção e status das variáveis do sistema.

---

## 🎮 Controles

### Movimentação e Câmera
* **W, A, S, D:** Movimentação direcional pelo espaço (Frente, Esquerda, Trás, Direita).
* **Mouse:** Rotação da câmera (Visão livre).
* **Botão Direito do Mouse:** Trava o cursor no centro da tela e assume o controle da visão.
* **Botão Esquerdo do Mouse:** Destrava o cursor, pausando o movimento da câmera para interagir com o SO.
* **Setas Direcionais (Cima, Baixo, Esquerda, Direita):** Rotação alternativa e manual da câmera.
* **Barra de Espaço:** Pular (aplica força de empuxo vertical sujeita à gravidade).
* **R:** Resetar a câmera, retornando o observador para a posição inicial da simulação.

### Controles do Sistema
* **P:** Alternar a câmera entre **Projeção Perspectiva** e **Projeção Ortográfica**.
* **L:** Ligar / Desligar a iluminação dinâmica.
* **T:** Ligar / Desligar a animação (congela o tempo e as órbitas dos planetas).
* **ESC:** Encerrar a aplicação.
