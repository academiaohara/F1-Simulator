# Guía del proyecto: qué hay, qué falta y cómo seguir

## 1. Conseguir el .exe (3 formas, de más fácil a menos)

1. **Sin instalar nada (recomendado):** sube el repo a GitHub. La acción `.github/workflows/build.yml`
   compila el `.exe` con MSVC en un Windows de GitHub y lo deja en *Actions → última ejecución → Artifacts →
   `f1_pixel_manager-windows`*. También ejecuta `make test`.
2. **Con Visual Studio Build Tools 2022** (componente "Desarrollo para el escritorio con C++"):
   doble clic en `build_windows.bat`. Detecta cualquier edición de Visual Studio con `vswhere`.
   Si usas PowerShell y te dice que los scripts están bloqueados, usa el `.bat` o
   `powershell -ExecutionPolicy Bypass -File .\build_windows.ps1`.
3. **Con MinGW-w64** (Windows: `winget install BrechtSanders.WinLibs.POSIX.UCRT`; Linux/macOS: paquete
   `mingw-w64`): `build_windows.bat` lo usa si no hay Visual Studio, o desde Linux `make windows`.

Resultado: `build\f1_pixel_manager.exe`. `package_windows.ps1` genera además `release\f1_pixel_manager.zip`.

> Qué estaba roto: `build_windows.ps1` solo buscaba las *Build Tools* (no Community/Pro), pasaba una línea
> con comillas anidadas a `cmd.exe` (PowerShell 5 las rompe), no funciona con la política de ejecución por
> defecto de Windows, y nunca se había probado. `Proyecto.zip` contenía solo binarios intermedios (`.obj`,
> `.exe` de depuración) y se ha eliminado. Verificado aquí: los 8 tests pasan y el .exe compila con MinGW;
> **la compilación con MSVC la valida la acción de GitHub** (no hay Windows en este entorno).

## 2. Arte (caras de pilotos y logos)

Está en `src/f1_art.c`, generado por código a partir de los datos, así que **cualquier piloto o equipo nuevo
tiene cara y logo sin tocar nada más**:

- **Cara:** tono de piel, pelo, barba y gafas salen de un hash del `id` del piloto; la camiseta usa los
  colores del equipo. Son retratos estilizados, **no fotos ni parecidos reales**.
- **Logo:** escudo con los colores del equipo, un motivo según el `id` y el código de 3 letras. No son los
  logos oficiales (marcas registradas).

Si quieres arte oficial o dibujado a mano, hay que añadir un cargador de PNG (por ejemplo `stb_image.h`, un
solo fichero) y que `f1_art_driver_portrait` / `f1_art_team_logo` busquen primero `assets/drivers/<id>.png`
y `assets/teams/<id>.png`, y usen el procedural si no existen. Ojo con los derechos de las imágenes reales.

## 3. Añadir un piloto o equipo nuevo

La fuente editable es `f1_pixel_manager.sqlite`; `src/f1_data_gen.c` se genera de ella:

```sh
python3 tools/export_f1_data.py f1_pixel_manager.sqlite src/f1_data_gen.c
make test
```

Campos de piloto: `id`, nombre, `code` (3 letras), nacionalidad, `team_id`, dorsal y atributos 0-100
(ritmo, clasificación, carrera, regularidad, neumáticos, lluvia, potencial). Equipo: `id`, nombre, `code`,
país, sede, motor, `color_primary`/`color_secondary` (0xRRGGBB) y reputación.
Límites en el código: 24 pilotos (`F1_MAX_DRIVERS`) y 12 equipos (`F1_MAX_TEAMS`) en `src/f1_game.h`.

## 4. Estado real: qué funciona y qué falta

Funciona: datos 2026, 24 circuitos reales, clasificación, carrera por sectores con IA, desarrollo del coche
(lógica), contratos/eventos (lógica), guardado con checksum, calendario multitemporada (la carrera siguiente
se prepara sola al acabar), hub/coche/desarrollo/carrera con datos reales del estado (esta versión).

Falta para que sea un **simulador de temporadas completo**, por prioridad:

1. **Clasificación de pilotos y fin de temporada**: el hub solo muestra constructores. Falta pantalla de
   campeonato de pilotos, resumen de fin de año (campeón, premios, dinero) y el reinicio entre temporadas
   (hoy `f1_complete_weekend` solo pone los puntos a cero).
2. **Pantalla de resultados** tras cada carrera (hoy salta directa a la siguiente).
3. **Evolución entre temporadas**: pilotos que mejoran/empeoran con `potential`, retiradas, novatos nuevos
   (aquí encaja "añadir los que vayan apareciendo": una tabla por año en la base SQLite), cambios de
   reglamento que reordenen los coches de la IA.
4. **Mercado de pilotos y contratos en la UI**: la lógica existe en `f1_team.c`, no hay pantalla.
5. **Desarrollo en la UI**: solo se puede iniciar el proyecto de *suelo* (`N`). Falta elegir componente,
   ver costes y el desarrollo de los rivales.
6. **Mejoras de simulación**: clasificación Q1/Q2/Q3 y libres (hoy un único cálculo), sprints
   (`F1_ROUNDS[].sprint` existe pero no se usa), banderas rojas, daños, y equilibrar con un lote estadístico
   (no debería ganar siempre el mismo).
7. **Modo "solo simular"** sin ver la carrera (ahora ~15 min por GP; x16 acelera pero no hay salto directo).
8. **Pulido**: sonido, menú inicial de elegir equipo (hoy `[`/`]`), idiomas, guardado de ajustes.
9. **Calidad**: el hub y las demás pantallas siguen fijas a 640×360; la ventana escala por enteros.

## 5. Orden de trabajo sugerido

Pantalla de resultados → campeonato de pilotos → fin de temporada → evolución de pilotos → UI de mercado
→ UI de desarrollo. Cada paso con su test en `tests/` (`make test` debe seguir en verde).

## 6. Comandos útiles

```sh
make test        # 8 suites headless
make previews    # regenera previews/*.png sin ventana
make windows     # .exe con MinGW desde Linux/macOS
```
