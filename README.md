# QuakePG Engine

## Sobre o projeto

QuakePG e uma game engine escrita do zero em C++17/OpenGL pra criar um FPS dungeon crawler roguelike medieval com visual PSX/retro. A arquitetura segue os principios do livro "Game Engine Architecture" de Jason Gregory.

O projeto e separado em duas partes:
- **engine/** - static lib (`libquakepg_engine.a`) com sistemas da engine
- **game/** - executavel (`quakepg_game`) do jogo

O client nunca inclui `<glad/glad.h>` ou `<GLFW/glfw3.h>` diretamente. Toda interacao com GPU/OS passa pela API da engine.

---

## Dependencias de sistema

```bash
# Ubuntu/Debian - pacotes necessarios pra compilar
sudo apt-get install build-essential cmake libx11-dev libxrandr-dev libxcursor-dev libxinerama-dev libxi-dev
```

GLFW e Dear ImGui ja estao em `vendor/` (bundled). Nao precisa instalar.

---

## Compilar e rodar

```bash
# Compilar
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Run (root do projeto, pra encontrar assets/)
cd ..
./quakepg_game
```

Controles: WASD mover, Mouse olhar, Shift sprint, ESC sair.

---

## Estrutura do projeto

```
quakepg/
├── engine/                     # Biblioteca estatica
│   ├── include/engine/         # Headers PUBLICOS (API da engine)
│   │   ├── engine.h            # Include tudo de uma vez
│   │   ├── core/               # Tipos, math, log, assert
│   │   ├── platform/           # Window, input, timer
│   │   ├── renderer/           # Shader, mesh, texture, camera, material
│   │   └── physics/            # Colisao AABB
│   └── src/                    # Implementacoes (PRIVADAS, nao incluir no game)
│
├── game/                       # Executavel do jogo
│   ├── include/game/           # Headers do game
│   │   └── dungeon/            # Sistema de mapas ASCII
│   └── src/                    # Codigo do game
│       ├── main.cpp            # Entry point + game loop
│       └── dungeon/            # Implementacao de mapas
│
├── assets/
│   ├── shaders/                # GLSL shaders
│   │   ├── psx.vert/frag       # Shader PSX (vertex snap, dither, fog)
│   │   └── basic.vert/frag     # Shader simples (sem efeitos PSX)
│   ├── variousretrotextures/   # Texturas retro prontas pra usar
│   ├── dungeon_tiles/          # Modelos 3D de dungeon (FBX)
│   ├── medievalweaponspack/    # Armas medievais (GLB)
│   └── knight/                 # Modelo de cavaleiro
│
└── vendor/                     # Dependencias
    ├── glad/                   # OpenGL loader
    ├── glfw/                   # Janela/input (bundled ou system)
    ├── imgui/                  # Dear ImGui (debug UI, editor tools)
    └── stb_image.h             # Carregamento de imagens
```

---

## Camadas da engine (de baixo para cima)

```
┌─────────────────────────────────────────┐
│             SEU JOGO (game/)            │  <-- voce escreve aqui
├─────────────────────────────────────────┤
│     Physics  │ Renderer  │ Resources    │  <-- subsistemas
├─────────────────────────────────────────┤
│     Platform (window, input, timer)     │  <-- abstrai SO/hardware
├─────────────────────────────────────────┤
│     Core (types, math, log, assert)     │  <-- fundacao
├─────────────────────────────────────────┤
│  Vendor (glad, GLFW, Dear ImGui, stb)   │  <-- libs externas
└─────────────────────────────────────────┘
```

Camadas de cima dependem das de baixo, nunca o contrario.

---

## Referencia

### Core: types.h

Typedefs pra nao escrever `uint32_t` toda hora.

```cpp
#include <engine/core/types.h>
using namespace qp;

u8, u16, u32, u64    // unsigned integers
i8, i16, i32, i64    // signed integers
f32, f64             // float, double
usize                // size_t
```

### Core: math.h

Matematica 3D. POD structs com funcoes livres (nao metodos).

```cpp
#include <engine/core/math.h>
using namespace qp;

// Vetores
Vec3 pos = {1.0f, 2.0f, 3.0f};
Vec3 dir = vec3(0, 0, -1);
Vec3 sum = pos + dir;
Vec3 scaled = pos * 2.0f;

f32 d = vec3_dot(a, b);
Vec3 c = vec3_cross(a, b);
Vec3 n = vec3_normalize(dir);
f32 len = vec3_length(pos);

// Matrizes (column-major, compativel com OpenGL)
Mat4 identity = mat4_identity();
Mat4 model = mat4_translate({1, 0, 0}) * mat4_rotate_y(to_radians(45)) * mat4_scale({2, 2, 2});
Mat4 view  = mat4_lookat(eye, target, {0,1,0});
Mat4 proj  = mat4_perspective(to_radians(90), 4.0f/3.0f, 0.1f, 50.0f);

// Utilidades
f32 rad = to_radians(90.0f);
f32 deg = to_degrees(QP_PI);
f32 v = clampf(x, 0.0f, 1.0f);
```

### Core: log.h / assert.h

Logging / Assert handling

```cpp
LOG_INFO("Player at %.1f %.1f %.1f", pos.x, pos.y, pos.z);
LOG_WARN("Low health: %d", hp);
LOG_ERROR("Failed to load: %s", path);
LOG_FATAL("Crash!"); // chama abort()

QP_ASSERT(ptr != nullptr);
QP_ASSERT_MSG(hp > 0, "Player is dead");
```

### Platform: window.h

Cria janela com contexto OpenGL. Wrapper do GLFW.

```cpp
WindowConfig cfg;
cfg.width = 960;
cfg.height = 720;
cfg.title = "Meu Jogo";
cfg.vsync = true;

Window* window = window_create(cfg);

while (!window_should_close(window)) {
    // game loop...
    window_swap_buffers(window);
}

window_destroy(window);
```

### Platform: input.h

Wrapper do teclado e mouse. 

```cpp
input_init(window);
input_set_cursor_locked(true); // FPS mode

// No loop:
input_update();  // salva estado anterior
input_poll();    // le estado novo

// Teclado
if (input_key_down(Key::W))       // segurando W
if (input_key_pressed(Key::Space)) // acabou de apertar (1 frame)
if (input_key_released(Key::E))   // acabou de soltar

// Mouse
f32 dx = input_mouse_dx(); // delta X desde ultimo frame
f32 dy = input_mouse_dy();
if (input_mouse_pressed(MouseButton::Left)) // clicou

// Teclas disponiveis:
// W A S D Q E R F Space LeftShift LeftCtrl Escape Tab
// Num1-Num5 Up Down Left Right
```

**Para adicionar novas teclas**: edite o enum `Key` em `engine/include/engine/platform/input.h` e a funcao `key_to_glfw()` em `engine/src/platform/input.cpp`.

### Platform: timer.h

```cpp
timer_init();

// No loop:
timer_update();
f64 dt = timer_delta();    // segundos desde ultimo frame (~0.016 a 60fps)
f64 total = timer_elapsed(); // segundos desde init
u32 fps = timer_fps();
```

### Renderer: shader.h

Da load e gerencia os shaders.

```cpp
// Carregar de arquivo
Shader s = shader_load("assets/shaders/psx.vert", "assets/shaders/psx.frag");

// Ou de string
Shader s = shader_create(vertex_source_str, fragment_source_str);

// Usar
shader_bind(s);
shader_set_mat4(s, "uModel", model.data);
shader_set_mat4(s, "uView", view.data);
shader_set_mat4(s, "uProjection", proj.data);
shader_set_float(s, "uSnapResolution", 160.0f);
shader_set_vec3(s, "uFogColor", 0.02f, 0.01f, 0.05f);
shader_set_int(s, "uDitheringEnabled", 1);
shader_set_vec4(s, "uTintColor", 1, 0, 0, 1); // vermelho
shader_unbind();

// Limpar
shader_destroy(s);
```

### Renderer: mesh.h

Geometria 3D. Cada vertice tem posicao, texcoord, normal, cor.

```cpp
// Layout do vertice:
struct Vertex {
    f32 position[3];   // location 0
    f32 texcoord[2];   // location 1
    f32 normal[3];     // location 2
    f32 color[4];      // location 3  (RGBA)
};

// Criar mesh de vertices + indices
Vertex verts[] = {
    {{0, 0.5f, 0}, {0.5f, 1}, {0,0,1}, {1, 0, 0, 1}}, // vermelho
    {{-0.5f, -0.5f, 0}, {0, 0}, {0,0,1}, {0, 1, 0, 1}}, // verde
    {{0.5f, -0.5f, 0}, {1, 0}, {0,0,1}, {0, 0, 1, 1}}, // azul
};
u32 indices[] = {0, 1, 2};
Mesh tri = mesh_create(verts, 3, indices, 3);

// Helpers prontos
Mesh tri  = mesh_create_triangle();
Mesh cube = mesh_create_cube();     // cubo 1x1x1 centrado na origem

// Desenhar
mesh_draw(cube);

// Limpar
mesh_destroy(cube);
```

**Para criar suas proprias meshes**: crie arrays de `Vertex` e `u32` indices, call `mesh_create()`. Veja `dungeon_map.cpp` para um exemplo de geracao procedural de geometria.

### Renderer: texture.h

Carrega texturas com GL_NEAREST (pixelado PSX).

```cpp
// Carregar de arquivo (PNG, JPG, BMP, TGA)
Texture tex = texture_load("assets/variousretrotextures/Brick_0.png");

// Textura 1x1 branca (quando nao quer textura, so vertex color)
Texture white = texture_create_white();

// Usar
texture_bind(tex, 0);  // slot 0
shader_set_int(shader, "uTexture", 0);
// ... desenhar ...
texture_unbind(0);

// Limpar
texture_destroy(tex);
```

ja tem algumas texturas PSX prontas em `assets/variousretrotextures/`:
- `Brick_0.png`, `Brick_1.png` 
- `Cobble.png`, `Cobble_Wall.png`, `Cobble_Ceiling.png`
- `Wood_0.png`, `Wood_Dark.png`
- `Metal_Plate.png` 

### Renderer: material.h

Agrupa shader + textura + cor num unico objeto.

```cpp
Material mat = material_create(psx_shader, brick_texture);
// ou sem textura (so vertex color):
Material mat = material_create_colored(psx_shader, {1, 0, 0, 1});

material_bind(mat);
mesh_draw(cube);
material_unbind();

material_destroy(mat);
```

### Renderer: camera.h

Camera FPS com yaw/pitch (sem roll).

```cpp
Camera cam;
cam.position = {0, 1.6f, 0};  // altura dos olhos
cam.fov = 90.0f;               // amplo (estilo Quake)
cam.speed = 4.0f;
cam.sensitivity = 0.15f;
cam.near_plane = 0.1f;
cam.far_plane = 50.0f;

// Mouse look (passa os deltas do mouse)
camera_process_mouse(cam, input_mouse_dx(), input_mouse_dy());

// Movimento (valores de -1 a 1 para cada eixo)
f32 fwd = 0, right = 0, up = 0;
if (input_key_down(Key::W)) fwd += 1;
if (input_key_down(Key::S)) fwd -= 1;
if (input_key_down(Key::D)) right += 1;
if (input_key_down(Key::A)) right -= 1;
camera_process_movement(cam, fwd, right, up, dt);

// Obter matrizes para o shader
Mat4 view = camera_view_matrix(cam);
Mat4 proj = camera_projection_matrix(cam, aspect_ratio);

// Direcao que a camera aponta
Vec3 forward = camera_forward(cam);
Vec3 right_dir = camera_right(cam);
```

### Renderer: renderer.h

Gerencia o framebuffer de resolucao interna (320x240px pra emular a do PSX) e faz upscale para a janela.

```cpp
RendererConfig cfg;
cfg.internal_width = 320;   // resolucao interna PSX
cfg.internal_height = 240;
Renderer* r = renderer_create(cfg);

renderer_set_clear_color(0.02f, 0.01f, 0.05f, 1.0f); // fundo escuro

// Cada frame:
renderer_begin_frame(r);   // bind FBO 320x240, limpa tela
// ... todos os seus draw calls aqui ...
renderer_end_frame(r);     // unbind FBO

// Upscale pra janela com GL_NEAREST (pixelado)
i32 w, h;
window_get_framebuffer_size(window, &w, &h);
renderer_present(r, w, h);

window_swap_buffers(window);

// Limpar
renderer_destroy(r);
```

### Physics: collision.h

Colisao AABB simples com slide (deslizar ao longo de paredes).

```cpp
AABB box_a = aabb_from_center_size({0, 1, 0}, {1, 2, 1});
AABB box_b = aabb_from_center_size({0.5f, 1, 0}, {1, 2, 1});

if (aabb_intersects(box_a, box_b)) {
    // colidiu!
}

// Slide collision: mover player sem atravessar paredes
AABB player = aabb_from_center_size(cam.position, {0.6f, 1.6f, 0.6f});
Vec3 velocity = move_dir * speed * dt;
Vec3 new_pos = aabb_slide(player, velocity, walls.data(), walls.size());
cam.position = new_pos;
```

### Dear ImGui

UI imediata pra debug, editor e ferramentas. Backend GLFW + OpenGL3 ja configurado.

```cpp
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Init (depois de criar janela e contexto GL)
ImGui::CreateContext();
ImGui_ImplGlfw_InitForOpenGL(window, true);
ImGui_ImplOpenGL3_Init("#version 330");

// No game loop, DEPOIS do renderer_end_frame (pra desenhar por cima):
ImGui_ImplOpenGL3_NewFrame();
ImGui_ImplGlfw_NewFrame();
ImGui::NewFrame();

// Suas janelas de debug aqui
ImGui::Begin("Debug");
ImGui::Text("FPS: %u", timer_fps());
ImGui::SliderFloat("Snap Resolution", &snap_res, 40.0f, 320.0f);
ImGui::SliderFloat("FOV", &cam.fov, 60.0f, 120.0f);
ImGui::ColorEdit3("Fog Color", fog_color);
ImGui::Checkbox("Dithering", &dithering_enabled);
ImGui::End();

// ImGui::ShowDemoWindow(); // janela demo com todos os widgets

ImGui::Render();
ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

// Cleanup (no shutdown)
ImGui_ImplOpenGL3_Shutdown();
ImGui_ImplGlfw_Shutdown();
ImGui::DestroyContext();
```

**Ordem no game loop**: ImGui deve renderizar DEPOIS do `renderer_end_frame()` e ANTES do `window_swap_buffers()`, pra desenhar na resolucao nativa da janela (nao na resolucao PSX 320x240).

**Input**: `ImGui_ImplGlfw_InitForOpenGL(window, true)` com `true` instala callbacks automaticamente. Se o ImGui estiver capturando input (`ImGui::GetIO().WantCaptureMouse`/`WantCaptureKeyboard`), nao processe input do jogo.

---

## Dungeon Maps

Cria dungeons com ASCII art no C++:

```cpp
#include <game/dungeon/dungeon_map.h>

static const char* MEU_MAPA[] = {
    "##########",
    "#...##...#",
    "#........#",
    "#..P.....#",
    "#........#",
    "#...##...#",
    "##########",
};
i32 rows = sizeof(MEU_MAPA) / sizeof(MEU_MAPA[0]);

DungeonMap dungeon;
dungeon_map_load(dungeon, MEU_MAPA, rows, 3.0f, 4.0f);
//                                        ^      ^
//                                  tamanho      altura
//                                da celula      das paredes

// Player spawn ('P' no mapa)
cam.position = dungeon.player_spawn;

// Renderizar
mesh_draw(dungeon.floor_mesh);
mesh_draw(dungeon.wall_mesh);
mesh_draw(dungeon.ceiling_mesh);

// Colisao
Vec3 new_pos = aabb_slide(player_box, velocity,
    dungeon.wall_colliders.data(), dungeon.wall_colliders.size());

// Limpar
dungeon_map_destroy(dungeon);
```

### Caracteres do mapa
| Char | Significado |
|------|-------------|
| `#`  | Parede (gera colisao + faces visiveis) |
| `.`  | Chao aberto (gera chao + teto) |
| `P`  | Spawn do player (tratado como `.`) |

### Importante
- A primeira e ultima linha/coluna devem ser `#` (paredes externas)
- Todas as linhas devem ter o mesmo comprimento
- So pode ter um `P`
- O sistema so gera faces de parede voltadas para espaco aberto (otimizado)
- Cor do chao tem variacao checkerboard sutil automatica

---

## PSX pipeline

O shader `psx.vert`/`psx.frag` simula as limitacoes do PlayStation 1:

### Vertex snapping
No PS1, coordenadas eram fixed-point(int). Vertices "tremiam" ao se mover e o vertex shader faz snap das posicoes para um grid:
```glsl
clipPos.xy = floor(clipPos.xy * uSnapResolution + 0.5) / uSnapResolution;
```
`uSnapResolution = 160.0` da um jitter sutil. Valores menores = mais jitter.

### Affine texture mapping
O PS1 nao fazia correcao de perspectiva nas texturas, causando distorcao, por isso usar `noperspective` no GLSL:
```glsl
noperspective out vec2 vTexCoord;
```

### Color depth reduction
PS1 tinha 15-bit colors (5 bits/canal = 32 levels). O fragment shader quantiza:
```glsl
color.rgb = floor(color.rgb * 31.0 + 0.5) / 31.0;
```

### Dithering
PS1 usava dithering pra disfarcar os poucos niveis de cor. Usamos uma Bayer matrix 4x4.

### Low-res render
Renderiza num FBO de 320x240, upscale pra janela com GL_NEAREST (sem suavizacao = pixelado).

### Fog
Linear fog por vertice, esconde o draw distance curto.

### Uniforms do shader PSX
| Uniform | Tipo | Descricao |
|---------|------|-----------|
| `uModel` | mat4 | Matriz de transformacao do objeto |
| `uView` | mat4 | Matriz da camera |
| `uProjection` | mat4 | Matriz de projecao |
| `uSnapResolution` | float | Grid de vertex snap (160.0 = sutil, 80.0 = forte, 0 = desligado) |
| `uFogColor` | vec3 | Cor do fog (combinar com clear color) |
| `uFogStart` | float | Distancia onde o fog comeca (default 5.0) |
| `uFogEnd` | float | Distancia onde o fog e 100% (default 40.0) |
| `uDitheringEnabled` | int | 0 = desligado, 1 = ligado |
| `uColorDepth` | float | Niveis de cor por canal (31.0 = PS1, 255.0 = full) |
| `uTexture` | sampler2D | Textura no slot 0 |
| `uUseTexture` | int | 0 = so vertex color, 1 = usa textura |
| `uTintColor` | vec4 | Multiplicador de cor |

---

## Debug UI (Dear ImGui)

Aperte **Tab** durante o jogo pra abrir o menu de debug. Tab de novo pra fechar e voltar ao jogo. O mouse e liberado quando o menu esta aberto. Se fechar pelo X do ImGui, o cursor volta automaticamente pro jogo.

### Controles disponiveis no menu

#### Performance
| Parametro | O que mostra |
|-----------|-------------|
| **FPS** | Frames por segundo |
| **Frame** | Tempo do frame em ms |
| **Pos** | Posicao do player no mundo |

#### Resolucao
| Parametro | O que faz | Faixa | Dica |
|-----------|-----------|-------|------|
| **Preset** | Resolucao interna pre-definida | 320x240, 512x384, 640x480, Native | PS1 = 320x240 |
| **Width/Height** | Resolucao interna customizada | 160-1920 / 120-1080 | Clique "Apply Resolution" pra aplicar |
| **Apply Resolution** | Recria o framebuffer com o novo tamanho | botao | Menor = mais pixelado, maior = mais definido |

A resolucao interna e o tamanho do FBO onde a cena e renderizada. Depois e feito upscale pra janela com GL_NEAREST (pixelado). Mudar a resolucao nao muda o tamanho da janela, so a qualidade interna.

#### Camera
| Parametro | O que faz | Faixa | Dica |
|-----------|-----------|-------|------|
| **FOV** | Campo de visao | 60-120 | 90 = Quake, 60 = cinematico, 120 = fisheye |
| **Speed** | Velocidade base do player | 1-20 | 4 = default |
| **Sprint Multiplier** | Multiplicador quando segura Shift | 1-5 | 2 = default (velocidade = speed * mult) |
| **Sensitivity** | Sensibilidade do mouse | 0.05-0.5 | 0.15 = default |

#### Vertex Snapping
| Parametro | O que faz | Faixa | Dica |
|-----------|-----------|-------|------|
| **Snap Resolution** | Grid de snap dos vertices | 0-320 | 0 = off, 80 = PS1 forte, 160 = sutil, 320 = quase nada |

Simula a precisao fixa do PS1 GTE. Vertices "tremem" ao se mover. Menor = mais jitter.

#### Fog
| Parametro | O que faz | Faixa | Dica |
|-----------|-----------|-------|------|
| **Fog Color** | Cor da nevoa | RGB | Combinar com Clear Color |
| **Fog Start** | Distancia onde o fog comeca | 0-50 | Menor = mais claustrofobico |
| **Fog End** | Distancia onde o fog e 100% | 1-100 | Menor = draw distance curto |

#### Color & Post-Processing
| Parametro | O que faz | Faixa | Dica |
|-----------|-----------|-------|------|
| **Clear Color** | Cor de fundo | RGB | Deve combinar com Fog Color |
| **Tint** | Multiplicador de cor global | RGBA | Vermelho = sangue, azul = frio |
| **Dithering** | Bayer dithering 4x4 | on/off | PS1 autentico = on |
| **Color Depth** | Niveis de cor por canal | 3-255 | 31 = PS1 (15-bit), 63 = 18-bit, 255 = full 24-bit |

Color Depth controla quantos niveis de cor cada canal (R/G/B) pode ter. O PS1 tinha 31 niveis (5 bits). Valores menores = mais posterizacao/banding. Combinado com dithering, valores baixos dao o look classico.

### Receitas de vibes

**Dungeon escuro classico (default)**
- Resolucao: 320x240
- Clear/Fog Color: `(0.02, 0.01, 0.05)` (roxo escuro)
- Fog Start: `5`, Fog End: `40`
- Snap: `160`, Dithering: on, Color Depth: `31`

**Catacumba claustrofobica**
- Resolucao: 320x240
- Clear/Fog Color: `(0.0, 0.0, 0.0)` (preto total)
- Fog Start: `2`, Fog End: `15`
- Snap: `80`, Dithering: on, Color Depth: `15`

**Ruinas ao luar**
- Resolucao: 512x384
- Clear/Fog Color: `(0.05, 0.07, 0.15)` (azul noturno)
- Fog Start: `10`, Fog End: `60`
- Snap: `0`, Dithering: off, Color Depth: `63`
- Tint: `(0.8, 0.85, 1.0, 1.0)` (tom frio)

**Inferno/lava**
- Resolucao: 320x240
- Clear/Fog Color: `(0.15, 0.02, 0.0)` (vermelho escuro)
- Fog Start: `3`, Fog End: `25`
- Snap: `120`, Dithering: on, Color Depth: `31`
- Tint: `(1.2, 0.7, 0.5, 1.0)` (quente)

**Clean moderno (sem efeitos PSX)**
- Resolucao: 640x480 ou Native
- Snap: `0`, Dithering: off, Color Depth: `255`
- Fog Start: `20`, Fog End: `80`
- Tint: `(1, 1, 1, 1)`

---

## Como adicionar coisas novas

### Adicionar uma nova tecla de input

1. Abra `engine/include/engine/platform/input.h`
2. Adicione no enum `Key`: ex. `G,`
3. Abra `engine/src/platform/input.cpp`
4. Adicione no `key_to_glfw()`: `case Key::G: return GLFW_KEY_G;`

### Adicionar um novo shader

1. Crie os `assets/shaders/meushader.vert` e `.frag`
2. No client: `Shader s = shader_load("assets/shaders/meushader.vert", "assets/shaders/meushader.frag");`
3. Vertex attributes sao fixos (position, texcoord, normal, color)

### Adicionar uma textura num objeto

```cpp
Texture brick = texture_load("assets/variousretrotextures/Brick_0.png");

// No loop de render:
shader_set_int(psx_shader, "uUseTexture", 1);
texture_bind(brick, 0);
mesh_draw(minha_mesh);
```

### Adicionar um novo tipo de celula no mapa

1. Veja `game/include/game/dungeon/dungeon_map.h`
2. Veja `game/src/dungeon/dungeon_map.cpp`
3. No `dungeon_map_load()`, adicione um case no parser. Exemplo pra `D` = porta:

```cpp
if (c == 'D') {
    // Tratar como chao aberto (gerar floor/ceiling)
    map.cells[z * map.width + x] = 'D';
    // Guardar posicao pra depois colocar uma mesh de porta
}
```

4. Na hora de checar se e parede: `dungeon_map_is_wall()` retorna false pro `D`

### Criar um novo subsistema na engine

Exemplo: sistema de audio.

1. Crie o header: `engine/include/engine/audio/audio.h`
2. Crie a implementacao: `engine/src/audio/audio.cpp`
3. Adicione a source em `engine/CMakeLists.txt`:
```cmake
set(ENGINE_SOURCES
    ...
    src/audio/audio.cpp
)
```
4. Adicione no export `engine/include/engine/engine.h`:
```cpp
#include <engine/audio/audio.h>
```
5. Compile: `cd build && cmake .. && make -j$(nproc)`

### Adicionar logica pro jogo

Toda logica especifica do jogo vai em `game/`. 
Exemplo: sistema de HP.

1. Crie `game/include/game/player/health.h`:
```cpp
#pragma once
#include <engine/core/types.h>

struct PlayerHealth {
    qp::i32 current = 100;
    qp::i32 max = 100;
};

void health_take_damage(PlayerHealth& hp, qp::i32 amount);
bool health_is_dead(const PlayerHealth& hp);
```

2. Crie `game/src/player/health.cpp` com a implementacao
3. Adicione `src/player/health.cpp` no `game/CMakeLists.txt`
4. Use no `main.cpp`

### Adicionar vendor deps

1. Coloque em `vendor/` (header-only ou submodule)
2. Se eh a engine q precisa: adicione include path em `engine/CMakeLists.txt` (PRIVATE)
3. Se eh o game q precisa: adicione include path em `game/CMakeLists.txt`

### Texturizar a dungeon

Hoje a dungeon renderiza so com vertex color (a textura branca 1x1 `white_tex`). Pra adicionar texturas reais nas paredes, chao e teto:

1. **Carregue as texturas** no `main.cpp`, antes do game loop:
```cpp
Texture wall_tex  = texture_load("assets/variousretrotextures/Brick_0.png");
Texture floor_tex = texture_load("assets/variousretrotextures/Cobble.png");
Texture ceil_tex  = texture_load("assets/variousretrotextures/Cobble_Ceiling.png");
```

2. **Ative o uso de textura no shader** antes de desenhar:
```cpp
shader_set_int(psx_shader, "uUseTexture", 1);
```

3. **Bind a textura certa antes de cada mesh**:
```cpp
texture_bind(floor_tex, 0);
mesh_draw(dungeon.floor_mesh);

texture_bind(wall_tex, 0);
mesh_draw(dungeon.wall_mesh);

texture_bind(ceil_tex, 0);
mesh_draw(dungeon.ceiling_mesh);
```

4. **Limpe as texturas no shutdown**:
```cpp
texture_destroy(wall_tex);
texture_destroy(floor_tex);
texture_destroy(ceil_tex);
```

5. **Ajuste os UVs se necessario**: o `push_quad()` em `dungeon_map.cpp` ja gera UVs de `(0,0)` a `(1,1)` por face. Se a textura ficar esticada (celulas grandes), voce pode escalar os UVs pra repetir a textura. Exemplo: trocar os `0, 0, 1, 1` no `push_quad` por `0, 0, cs, wh` (onde `cs` = cell_size, `wh` = wall_height). Isso faz a textura repetir proporcionalmente ao tamanho da parede. Pra funcionar, a textura precisa estar com `GL_REPEAT` (que ja e o default do OpenGL).

**Texturas disponiveis** em `assets/variousretrotextures/`:
`Brick_0.png`, `Brick_1.png`, `Cobble.png`, `Cobble_Wall.png`, `Cobble_Ceiling.png`, `Dead_Ground.png`, `Metal_Plate.png`, `Pipe.png`, `Tile.png`, `Wood_0.png`

Tambem tem texturas em `assets/dungeon_tiles/textures/` (1-bit pixel art style).

### Carregar e renderizar modelos 3D (armas, inimigos, props)

A engine ainda nao tem um loader de modelos 3D. Voce precisa implementar um pra poder usar os assets em `assets/medievalweaponspack/` (GLB), `assets/knight/` (GLB/FBX) e `assets/dungeon_tiles/` (FBX). Existem duas abordagens:

#### Opcao A: Usar Assimp (recomendado pra comecar)

Assimp e uma lib que le OBJ, FBX, GLB, GLTF e dezenas de outros formatos.

1. **Vendorize ou instale o Assimp**:
   - Sistema: `sudo apt-get install libassimp-dev`
   - Ou vendor: clone em `vendor/assimp` e adicione `add_subdirectory(vendor/assimp)` no CMake root

2. **Linke no CMake** (`engine/CMakeLists.txt`):
```cmake
target_link_libraries(quakepg_engine PRIVATE assimp)
```

3. **Crie o loader** em `engine/include/engine/resources/model.h`:
```cpp
#pragma once
#include <engine/renderer/mesh.h>
#include <engine/renderer/texture.h>
#include <vector>

namespace qp {

struct Model {
    std::vector<Mesh> meshes;
    std::vector<Texture> textures;
};

Model model_load(const char* path);
void  model_draw(const Model& m, const Shader& s);
void  model_destroy(Model& m);

} // namespace qp
```

4. **Implemente** em `engine/src/resources/model.cpp`:
   - Use `Assimp::Importer` pra abrir o arquivo
   - Itere `aiScene->mMeshes[]`, pra cada mesh:
     - Leia `mVertices[i]`, `mNormals[i]`, `mTextureCoords[0][i]` e converta pra `Vertex`
     - Leia `mFaces[j].mIndices[]` pra montar os indices
     - Chame `mesh_create(verts, count, indices, count)`
   - Pra texturas: leia `aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &path)` e chame `texture_load()`
   - GLB embute as texturas no arquivo, Assimp extrai elas automaticamente via `aiScene->mTextures[]`

5. **Use no game**:
```cpp
Model sword = model_load("assets/medievalweaponspack/MedievalWeaponsPack/Sword.glb");

// No render loop:
Mat4 weapon_model = mat4_translate(cam.position + camera_forward(cam) * 0.5f);
shader_set_mat4(psx_shader, "uModel", weapon_model.data);
model_draw(sword, psx_shader);

// Cleanup:
model_destroy(sword);
```

#### Opcao B: Loader minimalista so de OBJ

Se quiser evitar a dependencia do Assimp (ele e grande), pode implementar um parser de OBJ simples. OBJ e texto puro:

```
v 0.0 1.0 0.0       # vertice
vt 0.5 1.0           # texcoord
vn 0.0 0.0 1.0       # normal
f 1/1/1 2/2/2 3/3/3  # face (vertex/texcoord/normal indices, base-1)
```

1. Crie `engine/src/resources/obj_loader.cpp`
2. Parse linha por linha: `v` → push posicao, `vt` → push uv, `vn` → push normal, `f` → monte vertices indexados
3. Converta os indices pra `Vertex[]` + `u32[]` e chame `mesh_create()`
4. Pra usar os assets GLB/FBX, converta pra OBJ no Blender (File → Export → Wavefront OBJ)

#### Renderizar arma em primeira pessoa (viewmodel)

Depois de ter o loader de modelos:

1. Carregue o modelo da arma (ex: `Sword.glb`)
2. Renderize DEPOIS da dungeon, com uma matrix model especial:
```cpp
// Posicao relativa a camera (offset na frente + embaixo + lado)
Vec3 weapon_offset = camera_forward(cam) * 0.4f
                   + camera_right(cam) * 0.25f
                   + Vec3{0, -0.3f, 0};
Vec3 weapon_pos = cam.position + weapon_offset;

Mat4 weapon_model = mat4_translate(weapon_pos)
                  * mat4_rotate_y(to_radians(-cam.yaw - 90.0f))
                  * mat4_scale({0.5f, 0.5f, 0.5f}); // ajuste o scale

shader_set_mat4(psx_shader, "uModel", weapon_model.data);
shader_set_int(psx_shader, "uUseTexture", 1);
model_draw(sword, psx_shader);
```
3. **Dica**: pra evitar que a arma "entre" nas paredes, voce pode renderizar o viewmodel com depth buffer limpo (`glClear(GL_DEPTH_BUFFER_BIT)`) antes de desenhar a arma. Assim ela sempre fica na frente.

#### Renderizar inimigos/props no mundo

1. Carregue o modelo (ex: `knight.glb`)
2. Guarde a posicao de cada inimigo (ex: em um array ou futuro ECS)
3. No render loop, pra cada inimigo:
```cpp
Mat4 enemy_model = mat4_translate(enemy.position)
                 * mat4_rotate_y(to_radians(enemy.facing_angle))
                 * mat4_scale({1.0f, 1.0f, 1.0f});
shader_set_mat4(psx_shader, "uModel", enemy_model.data);
model_draw(knight, psx_shader);
```
4. Pra colisao, crie um AABB pro inimigo e use `aabb_intersects()` pra detectar hit

#### Assets disponiveis no projeto

| Pasta | Conteudo | Formato |
|-------|----------|---------|
| `assets/medievalweaponspack/MedievalWeaponsPack/` | Sword, Mace, Shield, Spear | GLB |
| `assets/knight/` | Modelo de cavaleiro com textura lowres | GLB, FBX |
| `assets/dungeon_tiles/` | Tiles de dungeon (paredes, chao, containers, etc.) | FBX |
| `assets/3D Retro Medieval Fantasy Kit/` | Kit medieval estilo retro | Variado |
| `assets/bigfoot/` | Modelo de bigfoot | Variado |
| `assets/Biblically Accurate Angel (Seraphim)/` | Anjo seraphim | Variado |
| `assets/PS1 Biblically Accurate Angel/` | Anjo estilo PS1 | Variado |

---

## Game loop - ordem importa

```cpp
while (!window_should_close(window)) {
    // 1. Tempo
    input_update();     // salva estado anterior do input
    timer_update();     // calcula delta time
    input_poll();       // le novo estado (glfwPollEvents)
    f32 dt = (f32)timer_delta();

    // 2. Input -> logica
    // Processar teclas, mouse look, etc.

    // 3. Update do jogo
    // Mover player, IA inimigos, fisica, combate
    // Usar fixed timestep pra fisica se precisar:
    //   accumulator += dt;
    //   while (accumulator >= FIXED_DT) { physics_step(); accumulator -= FIXED_DT; }

    // 4. Render
    renderer_begin_frame(renderer);     // bind FBO 320x240
    shader_bind(psx_shader);
    // set uniforms, draw meshes...
    shader_unbind();
    renderer_end_frame(renderer);       // unbind FBO

    // 5. Present
    renderer_present(renderer, fb_w, fb_h);  // upscale
    window_swap_buffers(window);              // vsync
}
```

---

## TODO's / Next steps


### Carregar modelos 3D
- Implementar loader de OBJ (ou FBX com lib tipo assimp)
- Voce ja tem modelos em `assets/dungeon_tiles/` e `assets/knight/`
- Criar `engine/include/engine/resources/resource_manager.h`

### Gameplay basico
- Player com HP, dano, morte
- Inimigos que patrulham e atacam
- Viewmodel de arma em primeira pessoa (os modelos estao em `assets/medievalweaponspack/`)
- HUD com barra de vida

### Geracao procedural
- Trocar o mapa hardcoded por geracao aleatoria
- Algoritmo sugerido: BSP tree (dividir area em salas) + conectar com corredores
- Ou: random walk

### Audio
- Integrar miniaudio (header-only, facil)
- Sons de passos, impacto de arma, ambiente de dungeon

### Editor
- ~~Integrar Dear ImGui~~ (feito - `vendor/imgui/`, backend GLFW+OpenGL3)
- Visualizar/editar mapas
- Ajustar parametros PSX em tempo real
- Inspector de entidades e componentes

---

## arquivos importantes

| Se tu quer.. | Arquivo |
|---|---|
| Mudar resolucao PSX | `game/src/main.cpp` → `RendererConfig` |
| Mudar FOV | `game/src/main.cpp` → `cam.fov` |
| Mudar sensibilidade do mouse | `game/src/main.cpp` → `cam.sensitivity` |
| Mudar velocidade do player | `game/src/main.cpp` → `cam.speed` |
| Mudar o mapa | `game/src/main.cpp` → `DUNGEON_MAP[]` |
| Mudar tamanho da celula | `dungeon_map_load()` → parametro `cell_size` |
| Mudar altura das paredes | `dungeon_map_load()` → parametro `wall_height` |
| Mudar cores do dungeon | `game/src/dungeon/dungeon_map.cpp` → `floor_color`, `wall_color`, etc. |
| Mudar fog | `assets/shaders/psx.vert` → `fogStart`, `fogEnd` |
| Ajustar todos os efeitos PSX | Aperte **Tab** no jogo → menu Dear ImGui |
| Mudar vertex jitter | Tab → Snap Resolution (0 = off, 80 = forte, 160 = sutil) |
| Mudar dithering | Tab → Dithering checkbox |
| Mudar fog | Tab → Fog Color, Fog Start, Fog End |
| Adicionar nova tecla | `engine/include/engine/platform/input.h` + `engine/src/platform/input.cpp` |
| Adicionar novo sistema na engine | Criar .h em `engine/include/engine/`, .cpp em `engine/src/`, add no CMake |
| Adicionar nova logica de game | Criar em `game/include/game/` e `game/src/`, add no CMake |
| Adicionar janela de debug ImGui | `game/src/main.cpp` → entre `renderer_end_frame` e `window_swap_buffers` |

---

## Convencoes

- **Namespace**: todo codigo da engine esta em `namespace qp`
- **Estilo**: C-with-structs. POD structs + funcoes livres. Sem heranca profunda.
- **Nomes**: `snake_case` para funcoes e variaveis, `PascalCase` para structs/enums
- **Prefixos**: funcoes do subsistema usam prefixo (`window_create`, `shader_bind`, `mesh_draw`)
- **Memoria**: a engine usa `new/delete` simples por enquanto. Quando precisar, implemente arena allocators em `core/memory.h`
- **Headers**: use `<engine/...>` para headers da engine, `<game/...>` para headers do game
