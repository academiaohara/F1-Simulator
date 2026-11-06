# F1 Pixel Manager

Mánager de Fórmula 1 en C con interfaz pixel art de 640×360. El proyecto mantiene la filosofía del Ciclismo Manager original: un solo `.exe` portable para Windows, sin instalador, servicios externos ni conexión a Internet.

## Estado de esta versión

Esta rama contiene la primera base jugable de la reconstrucción:

- Base 2026 integrada con 22 pilotos, 11 escuderías y 24 grandes premios.
- Trazados reales normalizados a partir del conjunto GeoJSON de `bacinger/f1-circuits` (MIT; licencia en `data/LICENSE-f1-circuits.md`).
- Clasificación determinista basada en piloto y coche.
- Simulación por sectores con combustible, ERS, cinco compuestos, degradación, lluvia, paradas, safety car, trompos, accidentes y adelantamientos.
- IA estratégica con estimaciones imperfectas que decide ritmo, ERS, neumáticos y ventanas de parada sin leer el futuro.
- Carreras rápidas: aproximadamente 14–15 minutos a x1, con pausa y velocidades x1, x2, x4, x8 y x16.
- Desarrollo de doce áreas del coche con presupuesto, ATR, capacidad simultánea, plazos y rendimientos decrecientes.
- Contratos, personal y eventos poco frecuentes con enfriamiento y máximo por temporada.
- Guardado versionado con checksum y escritura temporal.
- Pantallas pixel art de hub, monoplaza, desarrollo y carrera; repetición en primer plano para momentos importantes.

## Controles de la versión Windows

| Acción | Tecla |
|---|---|
| Hub / coche / desarrollo / carrera | `1` / `2` / `3` / `4` |
| Pausa o reanudar | `Espacio` |
| Bajar / subir velocidad | `←` / `→` |
| Elegir compuesto | `S` blando, `M` medio, `H` duro, `I` intermedio, `W` lluvia |
| Solicitar parada | `P` |
| Cambiar entre los dos pilotos | `Tab` |
| Cambiar escudería al iniciar desde el hub | `[` / `]` |
| Iniciar proyecto de suelo / instalar pieza lista | `N` / `Intro` en Desarrollo |
| Guardar / cargar la carrera | `F5` / `F9` |
| Probar ventana de repetición | `R` |
| Salir | `Esc` |

En la vista del coche, mueve el ratón sobre el monoplaza para destacar sus componentes.

## Compilar en Windows

Doble clic en `build_windows.bat` (Visual Studio/Build Tools o MinGW) o, sin instalar nada, descarga el `.exe`
de la pestaña *Actions* de GitHub. Detalles y solución de problemas en [`docs/GUIA.md`](docs/GUIA.md).

El ejecutable queda en `build\f1_pixel_manager.exe`.

Para generar una captura sin abrir la ventana:

```powershell
build\f1_pixel_manager.exe --shot preview.png race
```

Las vistas disponibles son `hub`, `car`, `development` y `race`.

## Pruebas de simulación

En Linux, macOS o un entorno con GCC:

```sh
make test
```

La suite cubre datos y trazados, calendario multitemporada, carrera, IA, desarrollo del coche, contratos y eventos, guardado y renderizado de las pantallas.

## Regenerar los datos integrados

La base SQLite es la fuente editable. El ejecutable no necesita SQLite.

```sh
python3 tools/enrich_f1_database.py "f1_pixel_manager.sqlite"
python3 tools/export_f1_data.py "f1_pixel_manager.sqlite" src/f1_data_gen.c
```

`tools/import_track_shapes.py` convierte el GeoJSON original a coordenadas enteras normalizadas almacenadas en `data/f1_tracks.json`.

## Qué falta

Ver [`docs/GUIA.md`](docs/GUIA.md): estado real, tareas pendientes por prioridad y cómo añadir pilotos/equipos.

## Estructura

- `src/f1_data*` — definiciones integradas de pilotos, equipos, circuitos y calendario.
- `src/f1_game*` — partida, clasificación y temporadas.
- `src/f1_race*` — simulación por sectores e incidentes.
- `src/f1_ai*` — decisiones estratégicas de los rivales.
- `src/f1_car*` — componentes y proyectos de desarrollo.
- `src/f1_team*` — contratos, personal y eventos.
- `src/f1_save*` — persistencia versionada.
- `src/f1_art*` — caras de pilotos y logos de escuderías generados por código.
- `src/f1_ui*`, `src/gfx*` — interfaz y framebuffer pixel art.
- `tests/` — ejecutables headless deterministas.

