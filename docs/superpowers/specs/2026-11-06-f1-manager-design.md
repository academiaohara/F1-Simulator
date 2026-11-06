# F1 Pixel Manager — Diseño

## Intención

Transformar Ciclismo Manager en un mánager de Fórmula 1 para Windows, completamente offline, portable y con pixel art. El jugador dirige una escudería real durante múltiples temporadas y actúa como jefe de estrategia durante fines de semana de carrera rápidos.

## Experiencia principal

- Elegir una escudería real y gestionar pilotos, personal, instalaciones, presupuesto y expectativas de la directiva.
- Desarrollar un monoplaza dividido en componentes, con vista explotada, foco visual, costes, plazos, riesgo y límites reglamentarios.
- Disputar entrenamientos, clasificación y carrera. El jugador ordena ritmo, ERS, neumáticos, paradas y órdenes de equipo; la IA conduce.
- Ver carreras condensadas con pausa y velocidades x1, x2, x4, x8 y x16. El juego reduce la velocidad ante decisiones importantes.
- Mostrar repeticiones pixel art en primer plano para salidas, adelantamientos, paradas disputadas, trompos, contactos y accidentes relevantes.
- Continuar durante varias temporadas con contratos, mercado, jóvenes ficticios, evolución competitiva y cambios reglamentarios.

## Simulación

Cada coche se simula por sectores y vueltas. El tiempo depende de piloto, coche, combustible, compuesto, edad del neumático, temperatura, pista, clima, tráfico, aire sucio, DRS, ERS, daños y errores. La IA usa predicciones imperfectas, evalúa ventanas de parada y reacciona a rivales, clima, coches de seguridad y órdenes de equipo. No conoce el futuro ni recibe ventajas ocultas.

Los incidentes usan tasas base pequeñas y modificadores visibles. Un accidente requiere una combinación plausible de presión, habilidad, lluvia, desgaste, daños o tráfico. Los eventos narrativos se limitan con enfriamientos y presupuestos de frecuencia para evitar temporadas caóticas.

## Datos

La distribución incluye pilotos, escuderías, motores, calendario y circuitos reales de la temporada base. SQLite es la fuente editable y un generador crea tablas C integradas en el ejecutable. Las partidas guardan una copia versionada del mundo para que una actualización del juego no altere temporadas empezadas.

Los circuitos incluyen longitud, vueltas, sectores, zonas DRS, pit lane, clima y una polilínea normalizada del trazado. Los retratos, coches, logos y escenas son pixel art originales inspirados en colores y rasgos reconocibles, sin depender de archivos externos en ejecución.

## Desarrollo del coche

Componentes iniciales: alerón delantero, alerón trasero, suelo, difusor, chasis, suspensión, refrigeración, frenos, caja de cambios, motor térmico, ERS y fiabilidad. Cada proyecto consume dinero, horas de túnel/CFD, capacidad de fábrica y tiempo de ingeniería. Existen rendimientos decrecientes, incertidumbre de fabricación, techo reglamentario, límite presupuestario y restricciones ATR según el campeonato anterior.

## Interfaz

Se conserva el framebuffer de 640×360 escalable, Win32/GDI y controles de ratón/teclado. El hub reúne calendario, mensajes, clasificación y estado del equipo. Las áreas principales son fin de semana, estrategia, coche, desarrollo, pilotos, personal, finanzas y reglamento.

La pantalla del coche muestra una vista lateral/explotada. Al pasar el cursor, resalta el componente y abre su ficha de rendimiento, desgaste, especificación instalada y proyectos posibles.

## Alcance técnico

- C nativo, Win32/GDI, sin dependencias en tiempo de ejecución.
- Un único `.exe` portable y guardado local.
- Pruebas headless deterministas para temporadas, carreras y estrategias.
- Primera entrega centrada en una temporada completa jugable; el pulido artístico y la profundidad de contenido crecerán por iteraciones.

## Enfoques considerados

1. Reescritura total: permitiría una arquitectura nueva, pero perdería el motor gráfico y elevaría mucho el riesgo.
2. Conversión directa de nombres y pantallas: sería rápida, pero conservaría supuestos ciclistas incorrectos.
3. Reutilizar plataforma y reemplazar dominio: conservar gráficos, ventana, entrada y utilidades; crear modelos F1 nuevos y migrar las pantallas gradualmente.

Se elige el tercero porque mantiene el formato solicitado y permite comprobar la simulación antes de invertir en todo el arte.

## Decisiones del constructor

- Carreras objetivo de 15–25 minutos a x1, con aceleración superior disponible.
- Datos base incluidos en el ejecutable y sin descarga por Internet.
- Simulación determinista mediante semilla guardada.
- Repeticiones breves que no detienen la carrera salvo que el jugador active esa preferencia.

