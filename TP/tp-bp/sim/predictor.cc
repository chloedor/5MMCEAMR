#include "predictor.h"

/////////////// STORAGE BUDGET JUSTIFICATION ////////////////

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

// Constructeur du prédicteur
PREDICTOR::PREDICTOR(char *prog, int argc, char *argv[])
{
   // La trace est tjs présente, et les arguments sont ceux que l'on désire
   if (argc != 2) {
      fprintf(stderr, "usage: %s <trace> pcbits countbits\n", prog);
      exit(-1);
   }

   uint32_t pcbits    = strtoul(argv[0], NULL, 0);
   uint32_t countbits = strtoul(argv[1], NULL, 0);
   hbits = 4;
   uint32_t hnumber = (1 << (hbits));
   perceptronsMask = (1 << (hbits+1)) - 1;
   hmask = hnumber - 1;

   ptype = GSHARE_PREDICTOR;

   nentries = (1 << pcbits);        // nombre d'entrées dans la table
   pcmask = (nentries - 1);       // masque pour n'accéder qu'aux bits significatifs de PC
   countmax = (1 << countbits) - 1; // valeur max atteinte par le compteur à saturation
   table = new uint32_t[nentries]();
   /*for(uint32_t i = 0; i < nentries; i++){
      table[i] = countmax / 2;
   }*/
   historyTable = new uint8_t[nentries]();
   doubleTable = new uint32_t*[hnumber];
   state = L0;
   y = 0;
   for (uint32_t i = 0; i < hnumber; ++i) {
      doubleTable[i] = new uint32_t[nentries]();
   }

   perceptronsTable = new int32_t*[nentries](); // tableau indexé par l'adresse du branchement
   for (int32_t i = 0; i < nentries; ++i) {
      perceptronsTable[i] = new int32_t[hbits+1](); // chaque entrée est un tableau de poids initialisé à 0
   }

   history = 0;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool PREDICTOR::GetPrediction(UINT64 PC)
{
   if(ptype == BIMODAL_PREDICTOR) {
      return GetPredictionBimodal(PC);
   }else if(ptype == GSHARE_PREDICTOR){
      return GetPredictionGshare(PC);
   } else if(ptype == LOCAL_PREDICTOR) {
      return GetPredictionLocalPredictor(PC);
   } else if (ptype == META_PREDICTOR){
      return GetPredictionMeta(PC);
   } else if (ptype == PERCEPTRON){
      return GetPredictionPerceptron(PC);
   } else {
      return GetPredictionBimodal(PC);
   }
   
}

bool PREDICTOR::GetPredictionBimodal(UINT64 PC)
{
   uint32_t v = table[PC & pcmask];
   return v > countmax / 2 ? TAKEN : NOT_TAKEN;
}

bool PREDICTOR::GetPredictionGshare(UINT64 PC)
{
   uint32_t v = table[(PC ^ ((history & hmask) | (~hmask))) & pcmask];
   return v > countmax / 2 ? TAKEN : NOT_TAKEN;
}

bool PREDICTOR::GetPredictionLocalPredictor(UINT64 PC)
{
   uint32_t h = historyTable[PC & pcmask];
   uint32_t v = doubleTable[h & hmask][PC & pcmask];
   return v > countmax / 2 ? TAKEN : NOT_TAKEN;
}

bool PREDICTOR::GetPredictionMeta(UINT64 PC)
{
   global = GetPredictionGshare(PC);
   local = GetPredictionLocalPredictor(PC);
   switch (state) {
      case G0:
         return global;
      case G1:
         return global;
      case L0:
         return local;
      case L1: 
         return local;
   }
   return global;
}

bool PREDICTOR::GetPredictionPerceptron(UINT64 PC)
{
   uint32_t index = PC & pcmask;
   y = perceptronsTable[index][0]; // y = w0 ajouté pour biaiser
   for(uint32_t i = 0; i < hbits; i++) {
      if((history & (1 << i)) == 0){ // si le (i+1) éme branchement de l'historique est non prit
         y -= (perceptronsTable[index][i+1] & perceptronsMask); // y = y - w(i+1)
      } else { // si le ième branchement de l'historique est prit
         y += (perceptronsTable[index][i+1] & perceptronsMask); // y = y + w(i+1)
      }
   }
   if(y >= 0){
      return TAKEN;
   }
   return NOT_TAKEN;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
   if(ptype == BIMODAL_PREDICTOR) {
      UpdatePredictorBimodal(PC, opType, resolveDir, predDir, branchTarget);
   } else if(ptype == GSHARE_PREDICTOR){
      UpdatePredictorGshare(PC, opType, resolveDir, predDir, branchTarget);
   } else if(ptype == LOCAL_PREDICTOR) {
      UpdatePredictorLocalPredictor(PC, opType, resolveDir, predDir, branchTarget);
   } else if(ptype == META_PREDICTOR) {
      UpdatePredictorMeta(PC, opType, resolveDir, predDir, branchTarget);
   } else if (ptype == PERCEPTRON){
      UpdatePredictorPerceptron(PC, opType, resolveDir, predDir, branchTarget);
   }
   else {
      UpdatePredictorBimodal(PC, opType, resolveDir, predDir, branchTarget);
   }
}

void PREDICTOR::UpdatePredictorBimodal(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
   uint32_t index = PC & pcmask;
   uint32_t v = table[index];
   table[index] = resolveDir == TAKEN ? SatIncrement(v, countmax) : SatDecrement(v);
   
}

void PREDICTOR::UpdatePredictorGshare(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
   uint32_t index = (PC ^ ((history & hmask) | (~hmask))) & pcmask;
   uint32_t v = table[index];
   table[index] = resolveDir == TAKEN ? SatIncrement(v, countmax) : SatDecrement(v);
   history = (history << 1) | resolveDir;
}

void PREDICTOR::UpdatePredictorLocalPredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
   uint32_t index = PC & pcmask;
   uint32_t historyIndex = historyTable[index] & hmask;

   uint32_t v = doubleTable[historyIndex][index];
   doubleTable[historyIndex][index] = resolveDir == TAKEN ? SatIncrement(v, countmax) : SatDecrement(v);
   historyTable[index] = (historyTable[index] << 1) | resolveDir;
}

void PREDICTOR::UpdatePredictorMeta(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
   switch (state) {
      case G0:
         UpdatePredictorGshare(PC, opType, resolveDir, predDir, branchTarget);
         if (local != resolveDir && global == resolveDir){
            state = G1;
         } else if (local == resolveDir && global != resolveDir){
            state = L0;
         }
         break;
      case G1:
         UpdatePredictorGshare(PC, opType, resolveDir, predDir, branchTarget);
         if (local == resolveDir && global != resolveDir){
            state = G0;
         }
         break;
      case L0:
         UpdatePredictorLocalPredictor(PC, opType, resolveDir, predDir, branchTarget);
         if (local != resolveDir && global == resolveDir){
            state = G0;
         } else if (local == resolveDir && global != resolveDir){
            state = L1;
         }
         break;
      case L1:
         UpdatePredictorLocalPredictor(PC, opType, resolveDir, predDir, branchTarget);
         if (local != resolveDir && global == resolveDir){
            state = L0;
         }
         break;
   }
}

void PREDICTOR::UpdatePredictorPerceptron(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
   int32_t theta = countmax;
   int32_t yout = 0;
   int32_t t = -1;
   uint32_t index = PC & pcmask;
   if(resolveDir){ // si le branchement est pris, t=1
      t = 1;
   }
   // Détermination de yout
   if(y > theta){
      yout = 1;
   } else if(y < -1*theta){
      yout = -1;
   }

   // Si l'algorithme a misspredict
   if(yout != t){
      perceptronsTable[index][0] += t; // Mise à jour de w0
      for(uint32_t i = 0; i<hbits; i++){
         if((history & (1 << i)) == 0) {
            perceptronsTable[index][i+1] -= t; // Mise à jour de w(i+1)
         } else {
            perceptronsTable[index][i+1] += t;
         }
      }
   }
   history = (history << 1) | resolveDir;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget)
{
   // This function is called for instructions which are not
   // conditional branches, just in case someone decides to design
   // a predictor that uses information from such instructions.
   // We expect most contestants to leave this function untouched.
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


/***********************************************************/
