#ifndef F1_ART_H
#define F1_ART_H

#include "f1_data.h"

/* Arte pixel generado por codigo: no hay ficheros de imagen que distribuir.
 * Todo se deriva de los datos (id del piloto, colores de la escuderia), asi que
 * un piloto o equipo nuevo en la base de datos obtiene cara y logo automaticamente. */

#define F1_PORTRAIT_W 30
#define F1_PORTRAIT_H 32
#define F1_LOGO_W 28
#define F1_LOGO_H 18

/* Cara 30x32 con el origen en (x, y). Determinista por id de piloto. */
void f1_art_driver_portrait(int x, int y, const F1DriverDef *driver);
/* Escudo 28x18 con el origen en (x, y). Determinista por id de escuderia. */
void f1_art_team_logo(int x, int y, const F1TeamDef *team);

#endif
