#include "tecnico.hpp"
#include "motorlib/util.h"
#include <iostream>
#include <queue>
#include <set>

using namespace std;

// =========================================================================
// ÁREA DE IMPLEMENTACIÓN DEL ESTUDIANTE
// =========================================================================

Action ComportamientoTecnico::think(Sensores sensores) {
  Action accion = IDLE;


  // Decisión del agente según el nivel
  switch (sensores.nivel) {
    case 0: accion = ComportamientoTecnicoNivel_0(sensores); break;
    case 1: accion = ComportamientoTecnicoNivel_1(sensores); break;
    case 2: accion = ComportamientoTecnicoNivel_2(sensores); break;
    //case 3: accion = ComportamientoTecnicoNivel_3(sensores); break;
    case 3: accion = ComportamientoTecnicoNivel_E(sensores); break; 
    case 4: accion = ComportamientoTecnicoNivel_4(sensores); break;
    case 5: accion = ComportamientoTecnicoNivel_5(sensores); break;
    case 6: accion = ComportamientoTecnicoNivel_6(sensores); break;
  }

  return accion;
}


// Niveles del técnico
Action ComportamientoTecnico::ComportamientoTecnicoNivel_0(Sensores sensores) {
  Action accion = IDLE;
  ActualizarMapa(sensores);

  if(sensores.superficie[0] == 'D'){
    tiene_zapatillas = true;
  }

  if(sensores.superficie[0] == 'U'){
    return IDLE; 
  }
  
  if (ultima_accion == TURN_SL or ultima_accion == TURN_SR)  giros++;
  else giros = 0;

  mapaExplorado[sensores.posF][sensores.posC] += 1; // Marcar la posición actual como explorada

  if (giros >4){ // Si se han dado muchos giros seguidos sin avanzar, es posible que estemos en un bucle){ 
    giros = 0;
    cambio = !cambio;
  }

  if ( sensores.agentes[2] == 'i' or sensores.agentes[1] == 'i' or sensores.agentes[3] == 'i'){
    ultima_accion = TURN_SL;
    return TURN_SL;
  }
  
  char i = ViablePorAltura(sensores.superficie[1], sensores.cota[1] - sensores.cota[0]);
  char c = ViablePorAltura(sensores.superficie[2], sensores.cota[2] - sensores.cota[0]);
  char d = ViablePorAltura(sensores.superficie[3], sensores.cota[3] - sensores.cota[0]);

  int pos = VeoCasillaInteresante_0(i, c, d, tiene_zapatillas);

  int posx = sensores.posF;
  int posy = sensores.posC;

  int x_frente = posx, y_frente = posy;
  int x_izq = posx, y_izq = posy;
  int x_der = posx, y_der = posy;

  if (sensores.rumbo == norte) x_frente--;
  else if(sensores.rumbo == noreste){ x_frente--; y_frente++; }
  else if(sensores.rumbo == este) y_frente++;
  else if(sensores.rumbo == sureste){ x_frente++; y_frente++; }
  else if(sensores.rumbo == sur) x_frente++;
  else if(sensores.rumbo == suroeste){ x_frente++; y_frente--; }
  else if(sensores.rumbo == oeste) y_frente--; 
  else if(sensores.rumbo == noroeste){ x_frente--; y_frente--; }

  // IZQUIERDA (aproximación: girar mentalmente)
  if (sensores.rumbo == norte){x_izq--; y_izq--;}
  else if(sensores.rumbo == noreste) x_izq--; 
  else if(sensores.rumbo == este) {x_izq--; y_izq++;} 
  else if(sensores.rumbo == sureste){ y_izq++; }
  else if(sensores.rumbo == sur){ x_izq++; y_izq++; }
  else if(sensores.rumbo == suroeste){  x_izq++; }
  else if(sensores.rumbo == oeste){ x_izq++; y_izq--; }
  else if(sensores.rumbo == noroeste){  y_izq--; }

  // DERECHA
  if (sensores.rumbo == norte){x_der--; y_der++;}
  else if(sensores.rumbo == noreste) y_der++; 
  else if(sensores.rumbo == este) {x_der++; y_der++;} 
  else if(sensores.rumbo == sureste){ x_der++; }
  else if(sensores.rumbo == sur){ x_der++; y_der--; }
  else if(sensores.rumbo == suroeste){  y_der--; }
  else if(sensores.rumbo == oeste){ x_der--; y_der--; }
  else if(sensores.rumbo == noroeste){  x_der--; }

  // visitas
  int v_frente = mapaExplorado[x_frente][y_frente];
  int v_izq    = mapaExplorado[x_izq][y_izq];
  int v_der    = mapaExplorado[x_der][y_der];

  if (pos != 0){
    if(pos == 2 && es_transitable(i) && v_izq < v_frente)
      pos = 1;
    else if(pos == 2 && es_transitable(d) && v_der < v_frente)
      pos = 3;
    else if(pos == 1 && es_transitable(c) && v_frente < v_izq)
      pos = 2;
    else if(pos == 1 && es_transitable(d) && v_der < v_izq)
      pos = 3;
    else if(pos == 3 && es_transitable(c) && v_frente < v_der)
      pos = 2;
    else if(pos == 3 && es_transitable(i) && v_izq < v_der)
      pos = 1;
  }

  switch (pos)
  {
  case 3:
    accion = TURN_SR;
    break;
  case 2:
    accion = WALK;
    break;
  case 1:
    accion = TURN_SL;
    break;
  default:
    if(cambio) accion = TURN_SR;
    else accion = TURN_SL;
    break;
  }


  ultima_accion = accion;
  return accion;
}

/**
 * @brief Determina la mejor oppcion entra las 3 casillas que tiene delante
 * @param i terreno que hay en la posicion 1 de la superficie 
 * @param c terreno que hay en la posicion 2 de la superficie 
 * @param d terreno que hay en la posicion 3 de la superficie
 * @return 2 si es mejor WALK, 3 si es mejor TURN_SR, 1 si es mejor TURN_SL, o nada interesante
 */
int ComportamientoTecnico::VeoCasillaInteresante_0(char i, char c, char d, bool zap) {
  if( c =='U') return 2;
  else if( i == 'U') return 1;
  else if( d == 'U') return 3;
  else if( c == 'C') return 2;
  else if( i == 'C') return 1;
  else if( d == 'C') return 3;

   return 0;
}

/**
 * @brief Determina casilla viable
 * @param casilla que se quiere comprobar
 * @param dif de altura entre la casilla actual y la casilla a comprobar
 * @param zap si se tiene la zapatillas puestas
 * @return 'P' si no es viable y la casilla en otro caso
 */
char ComportamientoTecnico::ViablePorAltura(char casilla, int dif){
  if(abs(dif)<= 1 and casilla != 'M' and casilla != 'B' ){
    return casilla;
  }
  else return 'P';
}

void ComportamientoTecnico::MarcarCasillasVisibles(Sensores sensores) {
    int f = sensores.posF;
    int c = sensores.posC;
    int maxF = mapaExplorado.size();
    int maxC = mapaExplorado[0].size();

    // Casilla actual
    mapaExplorado[f][c]++;

    // Las 8 casillas adyacentes
    for (int df = -1; df <= 1; df++) {
        for (int dc = -1; dc <= 1; dc++) {
            if (df == 0 && dc == 0) continue; // ya marcada arriba
            int nf = f + df;
            int nc = c + dc;
            if (nf >= 0 && nf < maxF && nc >= 0 && nc < maxC) {
                mapaExplorado[nf][nc]++;
            }
        }
    }
}

/**
 * @brief Comprueba si una celda es de tipo camino transitable.
 * @param c Carácter que representa el tipo de superficie.
 * @return true si es camino ('C'), zapatillas ('D') o meta ('U').
 */
bool ComportamientoTecnico::es_transitable(unsigned char c) const {
  return (c == 'C' ||  c == 'U');
}

/**
 * @brief Comprueba si una celda es de tipo camino transitable.
 * @param c Carácter que representa el tipo de superficie.
 * @return true si es camino ('C'), zapatillas ('D') o meta ('U').
 */
bool ComportamientoTecnico::es_camino(unsigned char c) const {
  return (c == 'C' || c == 'D' || c == 'U');
}


/**
 * @brief Comportamiento reactivo del técnico para el Nivel 1.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_1(Sensores sensores) {
  Action accion = IDLE;
  ActualizarMapa(sensores);

  mapaExplorado[sensores.posF][sensores.posC] ++; // Marcar la posición actual como explorada

  if(sensores.superficie[0] == 'D'){
    tiene_zapatillas = true;
  }

  if( ultima_accion == TURN_SL) giros++;
  else giros = 0;

  if(giros >= 7){
    bucle = true;
    giros = 0;
  }

  char adelante = ViablePorAltura(sensores.superficie[2],  sensores.cota[2] - sensores.cota[0]);

  Orientacion rumboi = sensores.rumbo;
  Orientacion rumbod = rumboi ;

  if (sensores.rumbo == norte){
    rumboi = noroeste;
    rumbod = noreste;
  }
  else if(sensores.rumbo == noreste){
    rumboi = norte;
    rumbod = este;
  }
  else if(sensores.rumbo == este){
    rumboi = noreste;
    rumbod = sureste;
  }
  else if(sensores.rumbo == sureste){
    rumboi = este;
    rumbod = sur;
  }
  else if(sensores.rumbo == sur){
    rumboi = sureste;
    rumbod = suroeste;
  }
  else if(sensores.rumbo == suroeste){
    rumboi = sur;
    rumbod = oeste;
  }
  else if(sensores.rumbo == oeste){
    rumboi = suroeste;
    rumbod = noroeste;
  }
  else if(sensores.rumbo == noroeste){
    rumboi = oeste;
    rumbod = norte;
  }

  char i = ViablePorAltura_1(sensores.superficie[1], sensores.cota[1] - sensores.cota[0],vecesVistas(sensores.posF, sensores.posC, rumboi));
  char c = ViablePorAltura_1(sensores.superficie[2],  sensores.cota[2] - sensores.cota[0], vecesVistas(sensores.posF, sensores.posC, sensores.rumbo));
  char d = ViablePorAltura_1(sensores.superficie[3],  sensores.cota[3] - sensores.cota[0], vecesVistas(sensores.posF, sensores.posC, rumbod));

  int pos = VeoCasillaInteresante_1(i, c, d);

  //printf (" Rumbo: %d, Superficie: %c, pos: %d  bucle: %d giros: %d\n", sensores.rumbo, sensores.superficie[0], pos, bucle, giros);
  //printf(" %d %d %d\n %d n %d\n %d %d %d\n\n", mapaExplorado[sensores.posF-1][sensores.posC-1], mapaExplorado[sensores.posF-1][sensores.posC], mapaExplorado[sensores.posF-1][sensores.posC+1], mapaExplorado[sensores.posF][sensores.posC-1], mapaExplorado[sensores.posF][sensores.posC+1], mapaExplorado[sensores.posF+1][sensores.posC-1], mapaExplorado[sensores.posF+1][sensores.posC], mapaExplorado[sensores.posF+1][sensores.posC+1]);

  switch (pos)
  {
  case 3:
    accion = TURN_SR;
    break;
  case 2:
    accion = WALK;
    break;
  default:
    if(bucle and adelante != 'P'){
      accion = WALK;
      bucle = false;
    }
    else accion = TURN_SL;
    break;
  }

  ultima_accion = accion;
  return accion;
}

/**
 * @brief Determina la mejor oppcion entra las 3 casillas que tiene delante
 * @param i terreno que hay en la posicion 1 de la superficie 
 * @param c terreno que hay en la posicion 2 de la superficie 
 * @param d terreno que hay en la posicion 3 de la superficie
 * @return 2 si es mejor WALK, 3 si es mejor TURN_SR, 1 si es mejor TURN_SL, o nada interesante
 */
int ComportamientoTecnico::VeoCasillaInteresante_1(char i, char c, char d){
  if ( c =='U') return 2;
  else if( i == 'U') return 1;
  else if( d == 'U') return 3;
  else if( c == 'C') return 2;
  else if( i == 'C') return 1;
  else if( d == 'C') return 3;
  else if( c == 'S') return 2;
  else if( i == 'S') return 1;
  else if( d == 'S') return 3;
  else if (c == 'H') return 2;
  else if (i == 'H') return 1;
  else if (d == 'H') return 3;
  else if (c == 'A') return 2;
  else if (i == 'A') return 1;
  else if (d == 'A') return 3;
  return 0;
}

char ComportamientoTecnico::ViablePorAltura_1(char casilla,  int dif,  int vecesExplorada){
  //printf("Casilla: %c, Dif: %d, VecesExplorada: %d\n", casilla,  dif, vecesExplorada);
  if(abs(dif)<= 1 and casilla != 'M' and casilla != 'B'){
    if(vecesExplorada > 0){
      return 'P';
    }
    else{
      return casilla;
    }
  }
  else return 'P';
}



int ComportamientoTecnico::vecesVistas(int posf, int posc, Orientacion brujula){

  if(brujula == norte) return mapaExplorado[posf-1][posc];
  if(brujula == noreste) return mapaExplorado[posf-1][posc+1];
  if(brujula == este) return mapaExplorado[posf][posc+1];
  if(brujula == sureste) return mapaExplorado[posf+1][posc+1];
  if(brujula == sur) return mapaExplorado[posf+1][posc];
  if(brujula == suroeste) return mapaExplorado[posf+1][posc-1];
  if(brujula == oeste) return mapaExplorado[posf][posc-1];
  if(brujula == noroeste) return mapaExplorado[posf-1][posc-1];

  return 0;
}



/**
 * @brief Mapa de vistos por el agente
 * @param posf Fila de la posición actual.
 * @param posc Columna de la posición actual.
 * @param brujula Orientación del agente.
 * @return Acción a realizar.
 */
void ComportamientoTecnico::ActualizarExploracion(int posf, int posc, int brujula){
 
  mapaExplorado[posf][posc] = 1; // Marcar la posición actual como explorada

  switch (brujula)
  {  
    case norte:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++)
          mapaExplorado[posf - j][posc + i] = 1; // Marcar las casillas delante como exploradas
      break;
    case noreste:
      mapaExplorado[posf - 1][posc] = 1;
      mapaExplorado[posf - 1][posc + 1] = 1;
      mapaExplorado[posf][posc + 1] = 1;
      mapaExplorado[posf - 2][posc] = 1;
      mapaExplorado[posf - 2][posc + 1] = 1;
      mapaExplorado[posf - 2][posc + 2] = 1;
      mapaExplorado[posf - 1][posc + 2] = 1;
      mapaExplorado[posf][posc + 2] = 1;
      break;
    case este:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++)          
          mapaExplorado[posf + i][posc + j] = 1; // Marcar las casillas delante como exploradas
      break;
    case sureste:
      mapaExplorado[posf][posc + 1] = 1;
      mapaExplorado[posf + 1][posc + 1] = 1;
      mapaExplorado[posf + 1][posc] = 1;
      mapaExplorado[posf][posc + 2] = 1;
      mapaExplorado[posf + 1][posc + 2] = 1;
      mapaExplorado[posf + 2][posc + 2] = 1;
      mapaExplorado[posf + 2][posc + 1] = 1;
      mapaExplorado[posf + 2][posc] = 1;
      break;
    case sur:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++)
          mapaExplorado[posf + j][posc - i] = 1; // Marcar las casillas delante como exploradas
      break;
    case suroeste:
      mapaExplorado[posf + 1][posc] = 1;
      mapaExplorado[posf + 1][posc - 1] = 1;
      mapaExplorado[posf][posc - 1] = 1;    
      mapaExplorado[posf + 2][posc] = 1;
      mapaExplorado[posf + 2][posc - 1] = 1;
      mapaExplorado[posf + 2][posc - 2] = 1;
      mapaExplorado[posf + 1][posc - 2] = 1;
      mapaExplorado[posf][posc - 2] = 1;   
      break;
    case oeste:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++)
          mapaExplorado[posf - i][posc - j] = 1; // Marcar las casillas delante como exploradas
      break;
    case noroeste:
      mapaExplorado[posf - 1][posc] = 1;
      mapaExplorado[posf - 1][posc - 1] = 1;
      mapaExplorado[posf][posc - 1] = 1;
      mapaExplorado[posf - 2][posc] = 1;
      mapaExplorado[posf - 2][posc - 1] = 1;
      mapaExplorado[posf - 2][posc - 2] = 1;
      mapaExplorado[posf - 1][posc - 2] = 1;
      mapaExplorado[posf][posc - 2] = 1;
      break;

    default:
      break;
  }
}


/**
 * @brief Comportamiento del técnico para el Nivel 2.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_2(Sensores sensores) {
  Action accion = IDLE;
  if (!hayPlan){
    // Invocar al método de búsqueda
    hayPlan = true;
  }
  if (hayPlan and plan.size()>0){
    accion = plan.front();
    plan.pop_front();
  }
  if (plan.size()== 0){
    hayPlan = false;
  }
  return accion;
}

/**
 * @brief Comportamiento del técnico para el Nivel 3.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_3(Sensores sensores) {
  Action accion = IDLE;
  if (!hayPlan){
    // Invocar al método de búsqueda
    hayPlan = true;
  }
  if (hayPlan and plan.size()>0){
    accion = plan.front();
    plan.pop_front();
  }
  if (plan.size()== 0){
    hayPlan = false;
  }
  return accion;
}

/**
 * @brief Comportamiento del técnico para el Nivel 4.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_4(Sensores sensores) {
  return IDLE;
}

/**
 * @brief Comportamiento del técnico para el Nivel 5.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_5(Sensores sensores) {
  return IDLE;
}

/**
 * @brief Comportamiento del técnico para el Nivel 6.
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_6(Sensores sensores) {
  return IDLE;
}

/**
 * @brief Comportamiento del técnico para el Nivel E (especial tutorial parte 2).
 * @param sensores Datos actuales de los sensores.
 * @return Acción a realizar.
 */
Action ComportamientoTecnico::ComportamientoTecnicoNivel_E(Sensores sensores){
  Action accion = IDLE;
  if (!hayPlan){
    // Invocar al método de búsqueda
    hayPlan = true;
  }
  if (hayPlan and plan.size()>0){
    accion = plan.front();
    plan.pop_front();
  }
  if (plan.size()== 0){
    hayPlan = false;
  }
  return accion;
}


//me he quedado por la pagina 12
/*
list<Action> ComportamientoTecnico::Busqueda_A(const EstadoT &inicio, const EstadoT &final,const vector<vector<unsigned char>> &terreno, const vector<vector<unsigned char>> &altura)
{
  NodoT current_node;
  list<NodoT> frontier;
  list<NodoT> explored;
  list<Action> plan;

  current_node.estado = inicio;
  frontier.push_back(current_node);
  bool SolutionFound = ( current_node.estado.site.f == final.site.f and current_node.estado.site.c == final.site.c );
  while ( !frontier.empty() and !SolutionFound )
  {

    frontier.pop_front();
    explored.push_back(current_node);

    //comprobamos si estoy en una casilla que da las zapatillas
    if (terreno[current_node.estado.site.f][current_node.estado.site.c] == 'D'){
      current_node.estado.zapatillas = true;
    }

    //generamos los hijos aplicar walk
    NodoT childWalk = current_node;
    childWalk.estado = applyT(WALK, current_node.estado, terreno, altura);
    if (childWalk.estado.site.f != final.site.f and childWalk.estado.site.c != final.site.c){
      childWalk.secuencia.push_back(WALK);
      current_node= childWalk;
      SolutionFound = true;
    }
    else if(!Find(childWalk, frontier) and !Find(childWalk, explored)){
      childWalk.secuencia.push_back(WALK);
      frontier.push_back(childWalk);
    }

    if(!SolutionFound){
      //hijo resultante aplicar giro izquierda
      NodoT childTurnSR = current_node;
      childTurnSR.estado = applyT(TURN_SR, current_node.estado, terreno, altura);
      if (!Find(childTurnSR, frontier) and !Find(childTurnSR, explored)){
        childTurnSR.secuencia.push_back(TURN_SR);
        frontier.push_back(childTurnSR);
      }

      //hijo resultante aplicar giro derecha
      NodoT childTurnSL = current_node;
      childTurnSL.estado = applyT(TURN_SL, current_node.estado, terreno, altura
      if(!Find(childTurnSL, frontier) and !Find(childTurnSL, explored)){
        childTurnSL.secuencia.push_back(TURN_SL);
        frontier.push_back(childTurnSL);
      }
    }

    if(!SolutionFound and !frontier.empty()){
      current_node = frontier.front();
      SolutionFound = ( current_node.estado.site.f == final.site.f and current_node.estado.site.c == final.site.c );
    }

    if(SolutionFound){
      plan = current_node.secuencia;
    }
  }

  return plan;
}*/

// =========================================================================
// FUNCIONES PROPORCIONADAS
// =========================================================================

/**
 * @brief Actualiza el mapaResultado y mapaCotas con la información de los sensores.
 * @param sensores Datos actuales de los sensores.
 */
void ComportamientoTecnico::ActualizarMapa(Sensores sensores) {
  mapaResultado[sensores.posF][sensores.posC] = sensores.superficie[0];
  mapaCotas[sensores.posF][sensores.posC] = sensores.cota[0];

  int pos = 1;
  switch (sensores.rumbo) {
    case norte:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++) {
          mapaResultado[sensores.posF - j][sensores.posC + i] = sensores.superficie[pos];
          mapaCotas[sensores.posF - j][sensores.posC + i] = sensores.cota[pos++];
        }
      break;
    case noreste:
      mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[1];
      mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[1];
      mapaResultado[sensores.posF - 1][sensores.posC + 1] = sensores.superficie[2];
      mapaCotas[sensores.posF - 1][sensores.posC + 1] = sensores.cota[2];
      mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[3];
      mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[3];
      mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[4];
      mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[4];
      mapaResultado[sensores.posF - 2][sensores.posC + 1] = sensores.superficie[5];
      mapaCotas[sensores.posF - 2][sensores.posC + 1] = sensores.cota[5];
      mapaResultado[sensores.posF - 2][sensores.posC + 2] = sensores.superficie[6];
      mapaCotas[sensores.posF - 2][sensores.posC + 2] = sensores.cota[6];
      mapaResultado[sensores.posF - 1][sensores.posC + 2] = sensores.superficie[7];
      mapaCotas[sensores.posF - 1][sensores.posC + 2] = sensores.cota[7];
      mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[8];
      mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[8];
      mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[9];
      mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[9];
      mapaResultado[sensores.posF - 3][sensores.posC + 1] = sensores.superficie[10];
      mapaCotas[sensores.posF - 3][sensores.posC + 1] = sensores.cota[10];
      mapaResultado[sensores.posF - 3][sensores.posC + 2] = sensores.superficie[11];
      mapaCotas[sensores.posF - 3][sensores.posC + 2] = sensores.cota[11];
      mapaResultado[sensores.posF - 3][sensores.posC + 3] = sensores.superficie[12];
      mapaCotas[sensores.posF - 3][sensores.posC + 3] = sensores.cota[12];
      mapaResultado[sensores.posF - 2][sensores.posC + 3] = sensores.superficie[13];
      mapaCotas[sensores.posF - 2][sensores.posC + 3] = sensores.cota[13];
      mapaResultado[sensores.posF - 1][sensores.posC + 3] = sensores.superficie[14];
      mapaCotas[sensores.posF - 1][sensores.posC + 3] = sensores.cota[14];
      mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[15];
      mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[15];
      break;
    case este:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++) {
          mapaResultado[sensores.posF + i][sensores.posC + j] = sensores.superficie[pos];
          mapaCotas[sensores.posF + i][sensores.posC + j] = sensores.cota[pos++];
        }
      break;
    case sureste:
      mapaResultado[sensores.posF][sensores.posC + 1] = sensores.superficie[1];
      mapaCotas[sensores.posF][sensores.posC + 1] = sensores.cota[1];
      mapaResultado[sensores.posF + 1][sensores.posC + 1] = sensores.superficie[2];
      mapaCotas[sensores.posF + 1][sensores.posC + 1] = sensores.cota[2];
      mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[3];
      mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[3];
      mapaResultado[sensores.posF][sensores.posC + 2] = sensores.superficie[4];
      mapaCotas[sensores.posF][sensores.posC + 2] = sensores.cota[4];
      mapaResultado[sensores.posF + 1][sensores.posC + 2] = sensores.superficie[5];
      mapaCotas[sensores.posF + 1][sensores.posC + 2] = sensores.cota[5];
      mapaResultado[sensores.posF + 2][sensores.posC + 2] = sensores.superficie[6];
      mapaCotas[sensores.posF + 2][sensores.posC + 2] = sensores.cota[6];
      mapaResultado[sensores.posF + 2][sensores.posC + 1] = sensores.superficie[7];
      mapaCotas[sensores.posF + 2][sensores.posC + 1] = sensores.cota[7];
      mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[8];
      mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[8];
      mapaResultado[sensores.posF][sensores.posC + 3] = sensores.superficie[9];
      mapaCotas[sensores.posF][sensores.posC + 3] = sensores.cota[9];
      mapaResultado[sensores.posF + 1][sensores.posC + 3] = sensores.superficie[10];
      mapaCotas[sensores.posF + 1][sensores.posC + 3] = sensores.cota[10];
      mapaResultado[sensores.posF + 2][sensores.posC + 3] = sensores.superficie[11];
      mapaCotas[sensores.posF + 2][sensores.posC + 3] = sensores.cota[11];
      mapaResultado[sensores.posF + 3][sensores.posC + 3] = sensores.superficie[12];
      mapaCotas[sensores.posF + 3][sensores.posC + 3] = sensores.cota[12];
      mapaResultado[sensores.posF + 3][sensores.posC + 2] = sensores.superficie[13];
      mapaCotas[sensores.posF + 3][sensores.posC + 2] = sensores.cota[13];
      mapaResultado[sensores.posF + 3][sensores.posC + 1] = sensores.superficie[14];
      mapaCotas[sensores.posF + 3][sensores.posC + 1] = sensores.cota[14];
      mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[15];
      mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[15];
      break;
    case sur:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++) {
          mapaResultado[sensores.posF + j][sensores.posC - i] = sensores.superficie[pos];
          mapaCotas[sensores.posF + j][sensores.posC - i] = sensores.cota[pos++];
        }
      break;
    case suroeste:
      mapaResultado[sensores.posF + 1][sensores.posC] = sensores.superficie[1];
      mapaCotas[sensores.posF + 1][sensores.posC] = sensores.cota[1];
      mapaResultado[sensores.posF + 1][sensores.posC - 1] = sensores.superficie[2];
      mapaCotas[sensores.posF + 1][sensores.posC - 1] = sensores.cota[2];
      mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[3];
      mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[3];
      mapaResultado[sensores.posF + 2][sensores.posC] = sensores.superficie[4];
      mapaCotas[sensores.posF + 2][sensores.posC] = sensores.cota[4];
      mapaResultado[sensores.posF + 2][sensores.posC - 1] = sensores.superficie[5];
      mapaCotas[sensores.posF + 2][sensores.posC - 1] = sensores.cota[5];
      mapaResultado[sensores.posF + 2][sensores.posC - 2] = sensores.superficie[6];
      mapaCotas[sensores.posF + 2][sensores.posC - 2] = sensores.cota[6];
      mapaResultado[sensores.posF + 1][sensores.posC - 2] = sensores.superficie[7];
      mapaCotas[sensores.posF + 1][sensores.posC - 2] = sensores.cota[7];
      mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[8];
      mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[8];
      mapaResultado[sensores.posF + 3][sensores.posC] = sensores.superficie[9];
      mapaCotas[sensores.posF + 3][sensores.posC] = sensores.cota[9];
      mapaResultado[sensores.posF + 3][sensores.posC - 1] = sensores.superficie[10];
      mapaCotas[sensores.posF + 3][sensores.posC - 1] = sensores.cota[10];
      mapaResultado[sensores.posF + 3][sensores.posC - 2] = sensores.superficie[11];
      mapaCotas[sensores.posF + 3][sensores.posC - 2] = sensores.cota[11];
      mapaResultado[sensores.posF + 3][sensores.posC - 3] = sensores.superficie[12];
      mapaCotas[sensores.posF + 3][sensores.posC - 3] = sensores.cota[12];
      mapaResultado[sensores.posF + 2][sensores.posC - 3] = sensores.superficie[13];
      mapaCotas[sensores.posF + 2][sensores.posC - 3] = sensores.cota[13];
      mapaResultado[sensores.posF + 1][sensores.posC - 3] = sensores.superficie[14];
      mapaCotas[sensores.posF + 1][sensores.posC - 3] = sensores.cota[14];
      mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[15];
      mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[15];
      break;
    case oeste:
      for (int j = 1; j < 4; j++)
        for (int i = -j; i <= j; i++) {
          mapaResultado[sensores.posF - i][sensores.posC - j] = sensores.superficie[pos];
          mapaCotas[sensores.posF - i][sensores.posC - j] = sensores.cota[pos++];
        }
      break;
    case noroeste:
      mapaResultado[sensores.posF][sensores.posC - 1] = sensores.superficie[1];
      mapaCotas[sensores.posF][sensores.posC - 1] = sensores.cota[1];
      mapaResultado[sensores.posF - 1][sensores.posC - 1] = sensores.superficie[2];
      mapaCotas[sensores.posF - 1][sensores.posC - 1] = sensores.cota[2];
      mapaResultado[sensores.posF - 1][sensores.posC] = sensores.superficie[3];
      mapaCotas[sensores.posF - 1][sensores.posC] = sensores.cota[3];
      mapaResultado[sensores.posF][sensores.posC - 2] = sensores.superficie[4];
      mapaCotas[sensores.posF][sensores.posC - 2] = sensores.cota[4];
      mapaResultado[sensores.posF - 1][sensores.posC - 2] = sensores.superficie[5];
      mapaCotas[sensores.posF - 1][sensores.posC - 2] = sensores.cota[5];
      mapaResultado[sensores.posF - 2][sensores.posC - 2] = sensores.superficie[6];
      mapaCotas[sensores.posF - 2][sensores.posC - 2] = sensores.cota[6];
      mapaResultado[sensores.posF - 2][sensores.posC - 1] = sensores.superficie[7];
      mapaCotas[sensores.posF - 2][sensores.posC - 1] = sensores.cota[7];
      mapaResultado[sensores.posF - 2][sensores.posC] = sensores.superficie[8];
      mapaCotas[sensores.posF - 2][sensores.posC] = sensores.cota[8];
      mapaResultado[sensores.posF][sensores.posC - 3] = sensores.superficie[9];
      mapaCotas[sensores.posF][sensores.posC - 3] = sensores.cota[9];
      mapaResultado[sensores.posF - 1][sensores.posC - 3] = sensores.superficie[10];
      mapaCotas[sensores.posF - 1][sensores.posC - 3] = sensores.cota[10];
      mapaResultado[sensores.posF - 2][sensores.posC - 3] = sensores.superficie[11];
      mapaCotas[sensores.posF - 2][sensores.posC - 3] = sensores.cota[11];
      mapaResultado[sensores.posF - 3][sensores.posC - 3] = sensores.superficie[12];
      mapaCotas[sensores.posF - 3][sensores.posC - 3] = sensores.cota[12];
      mapaResultado[sensores.posF - 3][sensores.posC - 2] = sensores.superficie[13];
      mapaCotas[sensores.posF - 3][sensores.posC - 2] = sensores.cota[13];
      mapaResultado[sensores.posF - 3][sensores.posC - 1] = sensores.superficie[14];
      mapaCotas[sensores.posF - 3][sensores.posC - 1] = sensores.cota[14];
      mapaResultado[sensores.posF - 3][sensores.posC] = sensores.superficie[15];
      mapaCotas[sensores.posF - 3][sensores.posC] = sensores.cota[15];
      break;
  }
}



/**
 * @brief Determina si una casilla es transitable para el técnico.
 * En esta práctica, si el técnico tiene zapatillas, el bosque ('B') es transitable.
 * @param f Fila de la casilla.
 * @param c Columna de la casilla.
 * @param tieneZapatillas Indica si el agente posee las zapatillas.
 * @return true si la casilla es transitable.
 */
bool ComportamientoTecnico::EsCasillaTransitableLevel0(int f, int c, bool tieneZapatillas) {
  if (f < 0 || f >= mapaResultado.size() || c < 0 || c >= mapaResultado[0].size()) return false;
  return es_camino(mapaResultado[f][c]);  // Solo 'C', 'S', 'D', 'U' son transitables en Nivel 0
}

/**
 * @brief Comprueba si la casilla de delante es accesible por diferencia de altura.
 * Para el técnico: desnivel máximo siempre 1.
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return true si el desnivel con la casilla de delante es admisible.
 */
bool ComportamientoTecnico::EsAccesiblePorAltura(const ubicacion &actual) {
  ubicacion del = Delante(actual);
  if (del.f < 0 || del.f >= mapaCotas.size() || del.c < 0 || del.c >= mapaCotas[0].size()) return false;
  int desnivel = abs(mapaCotas[del.f][del.c] - mapaCotas[actual.f][actual.c]);
  if (desnivel > 1) return false;
  return true;
}

/**
 * @brief Devuelve la posición (fila, columna) de la casilla que hay delante del agente.
 * Calcula la casilla frontal según la orientación actual (8 direcciones).
 * @param actual Estado actual del agente (fila, columna, orientacion).
 * @return Estado con la fila y columna de la casilla de enfrente.
 */
ubicacion ComportamientoTecnico::Delante(const ubicacion &actual) const {
  ubicacion delante = actual;
  switch (actual.brujula) {
    case 0: delante.f--; break;                        // norte
    case 1: delante.f--; delante.c++; break;     // noreste
    case 2: delante.c++; break;                     // este
    case 3: delante.f++; delante.c++; break;     // sureste
    case 4: delante.f++; break;                        // sur
    case 5: delante.f++; delante.c--; break;     // suroeste
    case 6: delante.c--; break;                     // oeste
    case 7: delante.f--; delante.c--; break;     // noroeste
  }
  return delante;
}


/**
 * @brief Imprime por consola la secuencia de acciones de un plan.
 *
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::PintaPlan(const list<Action> &plan)
{
  auto it = plan.begin();
  while (it != plan.end())
  {
    if (*it == WALK)
    {
      cout << "W ";
    }
    else if (*it == JUMP)
    {
      cout << "J ";
    }
    else if (*it == TURN_SR)
    {
      cout << "r ";
    }
    else if (*it == TURN_SL)
    {
      cout << "l ";
    }
    else if (*it == COME)
    {
      cout << "C ";
    }
    else if (*it == IDLE)
    {
      cout << "I ";
    }
    else
    {
      cout << "-_ ";
    }
    it++;
  }
  cout << "( longitud " << plan.size() << ")" << endl;
}



/**
 * @brief Convierte un plan de acciones en una lista de casillas para
 *        su visualización en el mapa 2D.
 *
 * @param st    Estado de partida.
 * @param plan  Lista de acciones del plan.
 */
void ComportamientoTecnico::VisualizaPlan(const ubicacion &st,
                                            const list<Action> &plan)
{
   listaPlanCasillas.clear();
  ubicacion cst = st;

  listaPlanCasillas.push_back({cst.f, cst.c, WALK});
  auto it = plan.begin();
  while (it != plan.end())
  {

    switch (*it)
    {
    case JUMP:
      switch (cst.brujula)
      {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, JUMP});
    case WALK:
      switch (cst.brujula)
      {
      case 0:
        cst.f--;
        break;
      case 1:
        cst.f--;
        cst.c++;
        break;
      case 2:
        cst.c++;
        break;
      case 3:
        cst.f++;
        cst.c++;
        break;
      case 4:
        cst.f++;
        break;
      case 5:
        cst.f++;
        cst.c--;
        break;
      case 6:
        cst.c--;
        break;
      case 7:
        cst.f--;
        cst.c--;
        break;
      }
      if (cst.f >= 0 && cst.f < mapaResultado.size() &&
          cst.c >= 0 && cst.c < mapaResultado[0].size())
        listaPlanCasillas.push_back({cst.f, cst.c, WALK});
      break;
    case TURN_SR:
      cst.brujula = (Orientacion) (( (int) cst.brujula + 1) % 8);
      break;
    case TURN_SL:
      cst.brujula = (Orientacion) (( (int) cst.brujula + 7) % 8);
      break;
    }
    it++;
  }
}


