# Practico 4 - Armado de la aeronave

Se compone un modelo unico de la aeronave (SIAI Marchetti/Aermacchi S-211) combinando
las primitivas del Practico 03 y se verifica la composicion rotandola alrededor de un
punto de referencia.

## Modulo Aircraft

El modelo no queda suelto en `main`: vive en `src/Scene/Aircraft/`.

- `init()` genera las piezas y calcula sus matrices locales una sola vez.
- `update(pos, angulos)` recalcula unicamente `pose_`.
- `collect(items)` agrega un `RenderItem` por pieza con la transformacion ya compuesta.

Formulas usadas:

```
Mpose        = T(pos) · R(angulos) · T(-ref)
Mmundo(pieza)= Mpose · Mlocal(pieza)
```

## Errores encontrados y correcciones

1. **Fuselaje no centrado.** `primitives::cylinder()` genera con base en `y = 0` y tapa
   en `y = largo`, no centrado en el origen. Con `largo = 2` y `R(-90°, X)` quedaba en
   `z in [-2, 0]` (nariz en -2, cola en 0), pero las demas piezas se ubicaron asumiendo
   un fuso centrado en +/-1, por lo que las aletas quedaban flotando fuera del fuso.
   **Correccion:** `M = T(0,0,1) · R(-90,X)` para centrarlo en `z in [-1, 1]` (en codigo:
   `translate` primero y despues `rotate`).

2. **Orden `translate`/`rotate` invertido (al intentar centrar).** GLM post-multiplica:
   `translate(I,v)` seguido de `rotate(m,a,ax)` da `M = T(v) · R(a)` (orienta y luego
   ubica), mientras que `rotate` seguido de `translate` da `R(a) · T(v)` y aplica `v` en
   el eje YA rotado. Con el orden equivocado, el `T(0,0,1)` del fuso caia sobre Y (el fuso
   subia) y el `T(0,0,-0.95)` de la nariz bajaba el cono. **Correccion:** usar `T · R`
   (`translate` y despues `rotate`) tanto en el fuso como en la nariz. La nariz queda con
   base en `z = -0.95` y apice en `z ~= -1.916` (hacia -Z, adelante).

3. **Empenaje horizontal asimetrico.** La aleta "izquierda" quedaba centrada en
   `x in [-0.25, 0.25]` y la "derecha" en `[0.2, 0.7]`. **Correccion:** simetricas con
   traslaciones opuestas en `x = -/+0.5` (sin escala negativa, como pide la guia).

4. **update() sin punto de referencia.** La consigna pide `T(pos) · R · T(-ref)`;
   faltaba el `T(-ref)`. **Correccion:** se agrego en `update()`.

5. **Makefile.** `Aircraft.cpp` no estaba en `LIB_DIRS` y no se compilaba/linkeba;
   ademas estaba duplicado. Se dejo una sola vez.

### Detalle de GLM

GLM **post-multiplica** en `translate` y `rotate` (`translate(m, v) = m · T`,
`rotate(m, a, ax) = m · R`), asi que el orden de las llamadas importa:

```
translate(I, v); rotate(m, a, ax);   ->  M = T(v) · R(a)   (ubica una pieza ya orientada)
rotate(I, a, ax); translate(m, v);   ->  M = R(a) · T(v)   (aplica v en el eje YA rotado)
```

Para ubicar una pieza rotada (fuso, nariz) se escribe **primero `translate` y despues
`rotate`** (`T · R`): orienta la primitiva y luego la coloca en el mundo.

## Despiece

| Parte | Malla | Matriz local |
|------|------|--------------|
| Fuselaje | cylinder(0.2, 2.0, 32) | T(0,0,1) · R(-90,X) |
| Nariz | cone(0.2, 45°, 32) | T(0,0,-0.95) · R(-90,X) |
| Ala izq / der | cube (reusada) | T(-/+0.65, -0.12, 0.1) ; S(1.25, 0.05, 0.7) |
| Aleta horiz izq / der | cube (reusada) | T(-/+0.35, 0.14, 0.75) ; S(0.5, 0.05, 0.4) |
| Timon vertical | cube (reusada) | T(0, 0.53, 0.7) ; S(0.05, 0.7, 0.35) |

Ninguna pieza simetrica usa escala negativa; se aplica traslacion opuesta.

Las piezas se apoyan en la **superficie** del fuso (radio 0.2), no en su eje: el ala baja
se ancla en `y=-0.12` (emerge por el vientre) y el empenaje/timon sobre el lomo. Ademas se
aplica una **escala global 0.75** a todas las matrices locales: como no hay camara, el
clip-space es el NDC `[-1,1]^3` y el fuso (largo 2) ya ocupaba `z=±1`, por lo que la nariz
y las puntas se recortaban; con la escala entran completas.

## Cuestiones a pensar

### Parte 1

- **collect() antes de update():** se usa `pose_` con su valor por defecto (identidad),
  el avion se dibuja en el origen sin rotacion. **Antes de init():** `piezas_` y `locales_`
  estan vacios, `collect()` agrega 0 items y no se dibuja nada (no crashea).
- **Separar init() de update():** `init()` genera geometria en CPU y la sube a la GPU
  (`glNamedBufferData`), costoso; hacerlo por cuadro reasignaria buffers miles de veces
  por segundo. `update()` solo arma una matriz de 16 floats, barato.
- **Reemplazar piezas por una malla .obj:** cambia solo `init()`. El resto del programa no
  cambia porque consume la lista de `RenderItem` ya transformados.

### Parte 2

- **Normal tras escalado no uniforme:** NO sigue siendo perpendicular si se la transforma
  como `M · n`. Hay que usar la inversa transpuesta de la matriz de modelo (lo que hace el
  vertex shader: `mat3(transpose(inverse(uModel)))`). En esta escena no afecta al render
  porque el fragment shader usa `uColor` (color plano) y no la normal.
- **Mallas distintas vs matrices locales:** 3 mallas distintas (cilindro, cono, cubo) y 7
  matrices locales. No coinciden porque varias piezas comparten la misma geometria (cubo)
  pero se ubican con transformaciones distintas (alas y cola).

### Parte 3

- **De que lado va `T(-ref)`:** a la derecha de la composicion (se aplica primero, en
  coordenadas del modelo): `T(pos) · R · T(-ref)`. Si se la pone del otro lado, la
  traslacion se rota junto con el avion y el punto de referencia no se lleva al origen
  antes de rotar: el avion pivota en un punto distinto y se corre de lugar.
- **Una pieza que se mueve sola (aleron):** no alcanza con la matriz local fija; hace falta
  una transformacion adicional por pieza (`pose · local · anim`).
- **Arbol de transformaciones:** no hace falta para este modelo; alcanza con lista de piezas
  y dos niveles (pose / local). DeJaria de alcanzar con piezas que dependen de otra (tren de
  aterrizaje, superficies de control) o jerarquia padre-hijo real.

## Como se sabe que esta bien

- Al aplicar la pose, todas las piezas giran juntas alrededor del mismo punto: ninguna se
  despega ni el conjunto se corre de lugar.
- Ninguna pieza simetrica usa escalado negativo.

## Build / run

```bash
make clean && make && ./bin/ogl-app
```

Correr desde la raiz del proyecto (el `ResourceManager` busca `shaders/` relativo al CWD).

Nota: el punto de referencia quedo en `(0,0,0)`. Si la figura de la guia indica otro punto,
se ajusta en `reference_point_` de `Aircraft`.
