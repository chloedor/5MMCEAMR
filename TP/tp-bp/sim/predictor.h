///////////////////////////////////////////////////////////////////////
////  Copyright 2015 Samsung Austin Semiconductor, LLC.                //
/////////////////////////////////////////////////////////////////////////
//
#ifndef _PREDICTOR_H_
#define _PREDICTOR_H_

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include "utils.h"

/* FP: Nombre de bits de PC utilisés pour indexer la table, utilisé pour définir la taille de la table */
/* FP: Nombre de bits par entrée la table: 1 => last value, 2 => bimodal, 3 => ... */

//NOTE competitors are allowed to change anything in this file

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

class PREDICTOR {

 /* Exemple simple : prédicteurs à saturation 
  *    une seule table indexée par une partie de pc
  *    valeur du compteur comprise entre 0 et countmax
  */
 private:
   uint32_t *table;   // table contenant les compteurs à saturation
   uint32_t **doubleTable; // table pour le local doubleTable[history][PC & pcmask]
   uint32_t pcmask;   // masque pour n'accéder qu'aux bits significatifs de PC
   uint32_t hmask;
   uint32_t perceptronsMask;
   uint32_t hbits;
   uint32_t history;  // historique global
   int32_t y;
   uint8_t *historyTable; //historique local [PC & pcmask]
   int32_t **perceptronsTable;
   PredictorType ptype;
   State state;
   bool local;
   bool global;

 public:
   uint32_t nentries; // nombre d'entrées dans la table
   uint32_t countmax; // valeur max atteinte par le compteur à saturation

   PREDICTOR(char *progname, int argc, char *argv[]);

   // The interface to the functions below CAN NOT be changed
   bool GetPrediction(UINT64 PC);
   void UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
   void TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget);

   // Contestants can define their own functions below

   bool GetPredictionBimodal(UINT64 PC);
   bool GetPredictionGshare(UINT64 PC);
   bool GetPredictionLocalPredictor(UINT64 PC);
   bool GetPredictionMeta(UINT64 PC);
   bool GetPredictionPerceptron(UINT64 PC);

   void UpdatePredictorBimodal(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
   void UpdatePredictorGshare(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
   void UpdatePredictorLocalPredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
   void UpdatePredictorMeta(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget);
   void UpdatePredictorPerceptron(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget); 
};
#endif
