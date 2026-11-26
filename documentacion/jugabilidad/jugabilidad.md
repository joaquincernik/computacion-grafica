# Jugabilidad - Camara de persecucion y ayudas de vuelo

Cambios de la rama `jugabilidad` respecto de `practico8`. El objetivo fue que el avion
se pueda volar con el teclado: la camara sigue al avion desde atras y los mandos dejan
de producir movimientos bruscos o descontrolados.

Archivos modificados:

- `src/CameraSystem/CameraSystem.h` y `.cpp`: camara detras del avion con retorno automatico.
- `src/InputHandler/InputHandler.h` y `.cpp`: mandos con retorno al trim, nivelador y amortiguador de guinada.
- `src/Scene/Aircraft/Aircraft.cpp`: pose del avion con los angulos del FDM en radianes.
- `src/main.cpp`: le pasa el estado del FDM al input y el `dt` a la camara; imprime datos de vuelo.

## 1. Pose del avion (`Aircraft::update`)

`to_world()` entrega `(phi, theta, psi)` en **radianes**, pero `update()` los convertia
otra vez con `glm::radians()` y ademas tomaba las componentes en otro orden (usaba
`angulos.y` como guinada). El avion no giraba como indicaba el FDM.

Antes:

```cpp
pose = glm::rotate(pose, glm::radians(angulos.y), vec3(0.0f, 1.0f, 0.0f)); // yaw (Y)
pose = glm::rotate(pose, glm::radians(angulos.x), vec3(1.0f, 0.0f, 0.0f)); // pitch (X)
pose = glm::rotate(pose, glm::radians(angulos.z), vec3(0.0f, 0.0f, 1.0f)); // roll (Z)
```

Despues:

```cpp
// angulos = (phi, theta, psi) en radianes, ya en ejes OpenGL (ver to_world)
// Mpose = T(pos) * R(yaw)*R(pitch)*R(roll) * T(-ref)
pose = glm::rotate(pose, angulos.z, vec3(0.0f, 1.0f, 0.0f)); // yaw   psi   (Y)
pose = glm::rotate(pose, angulos.y, vec3(1.0f, 0.0f, 0.0f)); // pitch theta (X)
pose = glm::rotate(pose, angulos.x, vec3(0.0f, 0.0f, 1.0f)); // roll  phi   (Z)
```

El orden guinada -> cabeceo -> alabeo es el de los angulos de Euler del FDM.

## 2. Camara detras del avion (`CameraSystem`)

En `practico8` la camara orbitaba libre con el mouse y no tenia en cuenta hacia donde
volaba el avion. Ahora tiene una **base** que sale del avion, y `yaw_`/`pitch_` pasan a
ser solo el desvio que agrega el mouse.

### Direccion de vuelo

La nariz del modelo apunta a `-Z` local. Aplicando `Ry(psi)·Rx(theta)` a `(0,0,-1)`:

```
adelante = ( -sin(psi)·cos(theta),  sin(theta),  -cos(psi)·cos(theta) )
```

El alabeo (`phi`) no cambia hacia donde apunta la nariz.

### Base detras del avion

La posicion de la camara se calcula en esfericas con `(cos p·cos y, sin p, cos p·sin y)`.
Para quedar detras, la parte horizontal tiene que ser `-adelante = (sin psi, cos psi)`,
y eso se cumple con `y = 90° - psi`:

```cpp
float yaw_base = radians(90.0f) - psi; // detrás del avión
float pitch_base = radians(10.0f);     // un poco desde arriba

float yaw_total = yaw_base + yaw_;
float pitch_total = clamp(pitch_base + pitch_, radians(-89.0f), radians(89.0f));

float x = distancia_ * cos(pitch_total) * cos(yaw_total);
float y = distancia_ * sin(pitch_total);
float z = distancia_ * cos(pitch_total) * sin(yaw_total);
```

El limite de ±89° ahora se aplica al angulo total y no solo a la parte del mouse.

### Retorno a la direccion de vuelo

`update()` recibe el `dt` del cuadro. Si el mouse no se mueve durante `espera_retorno_`
(1 s), el desvio se reduce un porcentaje por cuadro. Asi la camara arranca rapido y
llega suave:

```cpp
bool moviendo = (cmd.yaw_delta != 0.0f || cmd.pitch_delta != 0.0f);

if (moviendo)
{
    yaw_ += cmd.yaw_delta;
    pitch_ += cmd.pitch_delta;
    quieto_ = 0.0f;
}
else
{
    quieto_ += dt;
    if (quieto_ > espera_retorno_)
    {
        float k = glm::min(1.0f, velocidad_retorno_ * dt);
        yaw_ -= yaw_ * k;
        pitch_ -= pitch_ * k;
    }
}
```

El `min(1, ...)` evita pasarse de largo si un cuadro dura mucho.

### Mouse solo con boton derecho (`InputHandler::update`)

Antes cualquier movimiento del cursor movia la camara. Ahora solo cuenta con el boton
derecho apretado:

```cpp
mouse_movement(xpos_d, ypos_d);
if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
{
    m_cmd.yaw_delta = 0.0f;
    m_cmd.pitch_delta = 0.0f;
}
```

`mouse_movement` se sigue llamando siempre, para que `m_lastX`/`m_lastY` esten al dia y
la camara no salte al apretar el boton.

## 3. Mandos (`InputHandler::plane_movement`)

### Retorno al trim

En `practico8` los mandos quedaban donde se los dejaba, con un `rate` de 1 rad/s y un
limite de ±1 rad (57°), mucho mas de lo que acepta el FDM. Ahora:

- `rate` bajo a 0.3 rad/s.
- `set_controls()` guarda los mandos del trim en `m_trim`.
- Al soltar la tecla, cada mando vuelve al trim con `acercar()`, que avanza a lo sumo `paso`:

```cpp
static float acercar(float valor, float objetivo, float paso)
{
    float diferencia = objetivo - valor;           // cuánto falta
    return valor + clamp(diferencia, -paso, paso); // avanza a lo sumo 'paso'
}
```

Los `if` pasaron a `if / else if / else` para que el `else` haga el retorno.

### Elevador limitado alrededor del trim

Tirando del baston (flecha abajo) en un giro, el elevador llegaba al tope de -25° en
aproximadamente 1 s y el angulo de ataque subia a ~40°: el avion rolaba, se daba vuelta
y caia. Ahora solo se aparta ±5° del valor de trim (-5.4°):

```cpp
const float ELEV_RANGO = glm::radians(5.0f);
...
m_controls.elevator = clamp(m_controls.elevator, m_trim.elevator - ELEV_RANGO, m_trim.elevator + ELEV_RANGO);
```

### Nivelador de alas

El modelo no vuelve solo a nivel: si se lo inclina, queda inclinado. Cuando pasan
`m_espera_nivelar` (2 s) sin tocar las flechas, el alerón se mueve en contra de la
inclinacion (`phi`) y de la velocidad de rolido (`p`):

```cpp
else
{
    m_sin_alerones += dt;
    if (m_sin_alerones > m_espera_nivelar)
        m_controls.aileron = -K_PHI * estado.phi - K_P * estado.body_omega.x; // Nivelador
    else
        m_controls.aileron = acercar(m_controls.aileron, m_trim.aileron, rate * dt); // Alerón a neutro, mantiene la inclinación
}
```

Con `K_PHI = 0.3` y `K_P = 0.2`. Las flechas arriba/abajo tambien reinician la espera.
Durante esos 2 s el alerón vuelve a neutro y el avion mantiene la inclinacion, por lo
que se puede sostener un giro sin mantener la tecla.

### Timon en A/D con amortiguador de guinada

Las teclas del timon pasaron de Q/E a A/D. Al mantener el timon, la nariz iba y volvia
una vez por segundo (*dutch roll*, una oscilacion de guinada y alabeo poco amortiguada
en este modelo). Se separo lo que pide la tecla (`m_pedal`) de lo que recibe el FDM, y
se le suma la velocidad de guinada `r` para frenar ese vaiven:

```cpp
if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    m_pedal -= rate * dt; // Pedal derecho -> timón negativo -> nariz a la derecha
else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    m_pedal += rate * dt; // Pedal izquierdo -> timón positivo -> nariz a la izquierda
else
    m_pedal = acercar(m_pedal, m_trim.rudder, rate * dt);

m_pedal = clamp(m_pedal, -TIMON_MAX, TIMON_MAX);

// Amortiguador de guiñada: frena el vaivén de la nariz (dutch roll)
m_controls.rudder = m_pedal + K_R * estado.body_omega.z;
```

Con `K_R = 0.5`. El signo es `+` porque en el modelo `Cn_delta_r < 0`: timon positivo
frena una guinada positiva.

### Estado del FDM en el input

Para el nivelador y el amortiguador, `update()` y `plane_movement()` reciben el estado
del FDM (en ejes NED, asi los signos coinciden con los de los mandos):

```cpp
void update(GLFWwindow *window, float dt, const dlfdm::AircraftState &estado);
```

Miembros nuevos en `InputHandler.h`: `m_trim`, `m_sin_alerones`, `m_espera_nivelar` y `m_pedal`.

## 4. `main.cpp`

```cpp
input_handler.update(ctx.window, frame_dt, fdm.getState());
...
camera_system.update(posicion, angulos, cmd, static_cast<float>(frame_dt));
```

Ademas imprime en la terminal la velocidad, la altura y la potencia dos veces por
segundo. No se usa el titulo de la ventana porque en Wayland, con libdecor desactivado,
GLFW dibuja una barra sin texto:

```cpp
static double proximo_print = 0.0;
if (current_frame_time >= proximo_print)
{
    const dlfdm::AircraftState &s = fdm.getState();
    cout << "V: " << (int)glm::length(s.body_velocity) << " m/s"
         << "  Alt: " << (int)-s.inertial_position.z << " m"
         << "  Pot: " << (int)(input_handler.controls().throttle * 100) << "%" << endl;
    proximo_print = current_frame_time + 0.5;
}
```

La altura es sobre el nivel del mar (la del FDM). El terreno esta trasladado a
`altura_terreno = 3500` m, asi que la altura sobre el suelo es aproximadamente `Alt - 3500`.

## Pendientes

- `ALERON_MAX` sigue en 20°: un segundo de flecha deja el avion casi invertido. Con 5° es controlable.
- `TIMON_MAX` sigue en 8°: con 3° el giro con timon queda mas suave.
- La potencia vuelve al trim al soltar W/S; deberia quedar donde se la deja, como una palanca.
- El amortiguador de guinada tambien frena los giros sostenidos (a 28° de inclinacion gira a
  ~1.5°/s en vez de ~2°/s). Se puede corregir con un filtro *washout* o bajando `K_R`.
- En `CameraSystem::update` se calcula `adelante` pero todavia no se usa, y el comentario de
  `angulos_avion` en el `.h` dice que no se usa.
