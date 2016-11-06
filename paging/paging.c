/*
* Yan Huo
* Dec 4th, 2015
*
* I tried two strategies paging:
*  1) Least-recently-used (LRU): swap out the page that's least recently accessed.
*  2) Predictive: swap out the page that you predict will not be accessed for the longest time. 
*
* And the results show that the Predictive Strategy is better.
* The code for the first strategy was commented out and shown at the bottom of my submission.
*/



#include <stdio.h> 
#include "t4.h"

int history [MAXPROCESSES][MAXPROCPAGES][MAXPROCPAGES]; // store the history for prediction.
int lastOne [MAXPROCESSES];
int initialized = 0; // the history table has not been initialized
 
struct ProcPagePair  {
   int proc;
   int page;
};


void pageit(Pentry q[MAXPROCESSES]) { 
   int  pc, proc, page, curPage, nextPage, lastPage, cur, nxt;
   struct ProcPagePair currentList [MAXPROCESSES]; // store the current pages
   struct ProcPagePair nextList [MAXPROCESSES * MAXPROCPAGES]; // store the potential next pages
   struct ProcPagePair canSwapOutList [MAXPROCESSES * MAXPROCPAGES]; // store the pages can be swapped out
   int numOfCurrent = 0;
   int numOfNext = 0;
   int numOfCanSwapOut = 0;

   // if not initialized, initialize the history and lastOne table.
   if (!initialized) { 
      int i, j, k;
      for (i = 0; i < MAXPROCESSES; i++) {
         lastOne[i] = -1;
         for (j = 0; j < MAXPROCPAGES; j++) {
            for (k = 0; k < MAXPROCPAGES; k++) {
               history[i][j][k] = 0;
            }
         }
      }
      initialized = 1;
   }

   // update the history
   for (proc = 0; proc < MAXPROCESSES; proc++) {
      curPage = q[proc].pc / PAGESIZE;
      lastPage = lastOne[proc];
      if (curPage != lastPage) {
         lastOne[proc] = curPage;
         if (lastPage != -1) { // if the last page exists
            history [proc][lastPage][curPage] = 1;  
         }      
      }
   }



   // update the three lists
   for (proc = 0; proc < MAXPROCESSES; proc++) {
      if (q[proc].active) {
         pc = q[proc].pc;
         curPage = pc / PAGESIZE;

         // update the current list
         currentList[numOfCurrent].proc = proc;
         currentList[numOfCurrent].page = curPage;
         numOfCurrent++;

         // update the potential next list
         for (nextPage = 0; nextPage < MAXPROCPAGES; nextPage++) {
            if (history[proc][curPage][nextPage] == 1) {
               nextList[numOfNext].proc = proc;
               nextList[numOfNext].page = nextPage;
               numOfNext++;
            }
         }

         // update the canSwapOut list
         for (page = 0; page < q[proc].npages; page++) {
            if (!q[proc].pages[page]) { // if the page has not been swapped in, we can continue.
               continue;
            }     
            int currentOrNext = 0;
            // check if in currentList
            for (cur = 0; cur < numOfCurrent; cur++) {
               if (currentList[cur].proc == proc) {
                  if (currentList[cur].page == page) {
                     currentOrNext = 1;
                     break;
                  }
               }
            }

            // check if in nextList
            for (nxt = 0; nxt < numOfNext; nxt++) {
               if (currentOrNext) {
                  break;
               }
               if (nextList[nxt].proc == proc) {
                  if (nextList[nxt].page == page) {
                     currentOrNext = 1;
                     break;
                  }
               }
            }

            // if not in currentList or nextList, add into CanSwapOutList
            if (!currentOrNext) {
               canSwapOutList[numOfCanSwapOut].proc = proc;
               canSwapOutList[numOfCanSwapOut].page = page;
               numOfCanSwapOut++;
            }           
         }
      }
   }

   // try to swap in the cur pages
   for (cur = 0; cur < numOfCurrent; cur++) {
      int currentProc = currentList[cur].proc;
      int currentPage = currentList[cur].page;
      if (q[currentProc].pages[currentPage]) { // if the page is already in, we don't need to swap in
         continue;
      }
      if (!pagein(currentProc, currentPage)) { // we need to swap out some page
         if (numOfCanSwapOut > 0) {
            pageout(canSwapOutList[numOfCanSwapOut-1].proc, canSwapOutList[numOfCanSwapOut-1].page);
            numOfCanSwapOut--;
         }
         else { // we don't have enough pages to swap out
            break;
         }
      }    
   }

   // try to swap in the next pages
   for (nxt = 0; nxt < numOfNext; nxt++) {
      int nextProc = nextList[nxt].proc;
      int nextPage = nextList[nxt].page;
      if (q[nextProc].pages[nextPage]) { // if the page is already in, we don't need to swap in
         continue;
      }
      if (!pagein(nextProc, nextPage)) { // we need to swap out some page
         if (numOfCanSwapOut > 0) {
            pageout(canSwapOutList[numOfCanSwapOut-1].proc, canSwapOutList[numOfCanSwapOut-1].page);
            numOfCanSwapOut--;
         }
         else { // we don't have enough pages to swap out
            break;
         }
      }   
   }
}

/* The code for LRU strategy: 
The predictive strategy gives better results, I did not turn in this one.

void pageit(Pentry q[MAXPROCESSES]) {
   static int tick=0; // artificial time
   static int timestamps[MAXPROCESSES][MAXPROCPAGES];
   static int usedPagePool = 0; 
   int proc,pc,page,oldpage; 
   for (proc=0; proc<MAXPROCESSES; proc++) {
      if (q[proc].active) { 
         pc=q[proc].pc;
         page = pc/PAGESIZE;
         timestamps[proc][page] = tick; 
         if (!q[proc].pages[page]) { 
            if (pagein(proc,page)) {
               usedPagePool++;
            }
            else {
               int pageIndex;
               int markLRU;
               int mark = 1;
               for(pageIndex = 0; pageIndex < MAXPROCPAGES; ++pageIndex){
                  if(mark){
                     if(q[proc].pages[pageIndex]){
                        markLRU = pageIndex;
                        mark = 0;
                     }
                  }
                  else {
                     if(q[proc].pages[pageIndex]){
                        if(timestamps[proc][pageIndex] < timestamps[proc][markLRU]){
                           markLRU = pageIndex;
                        }
                     }
                  }
               } 
               if (mark) {
                  continue;
               }
               if (pageout(proc,markLRU)) {
                  usedPagePool--;
               }
               else {
                  continue;         
               }
            } 
         } 
      }  
   }  
   tick++; 
} 
*/




