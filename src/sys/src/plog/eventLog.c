/* $Id: eventLog.c,v 1.3 2000/08/16 05:14:09 knepley Exp $ */

#include "petsc.h"        /*I    "petsc.h"   I*/
#include "src/sys/src/plog/ptime.h"
#include "plog.h"

/* Variables for the tracing logger */
extern FILE          *tracefile;
extern int            tracelevel;
extern char          *traceblanks;
extern char           tracespace[128];
extern PetscLogDouble tracetime;

/*------------------------------------------------ General Functions ------------------------------------------------*/
#undef __FUNCT__  
#define __FUNCT__ "EventLogDestroy"
/*
  EventLogDestroy - This destroys a EventLog object.

  Not collective

  Input Paramter:
. eventLog - The EventLog

  Level: beginner

.keywords: log, event, destroy
.seealso: EventLogCreate()
*/
int EventLogDestroy(EventLog eventLog)
{
  int event;
  int ierr;

  PetscFunctionBegin;
  for(event = 0; event < eventLog->numEvents; event++) {
    ierr = PerfInfoDestroy(&eventLog->eventInfo[event]);                                                  CHKERRQ(ierr);
  }
  ierr = PetscFree(eventLog->eventInfo);                                                                  CHKERRQ(ierr);
  ierr = PetscFree(eventLog);                                                                             CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "EventLogCopy"
/*
  EventLogCopy - This copys an EventLog object.

  Not collective

  Input Parameter:
. eventLog - The EventLog

  Output Parameter:
. newLog   - The copy

  Level: beginner

.keywords: log, event, copy
.seealso: EventLogCreate(), EventLogDestroy()
*/
int EventLogCopy(EventLog eventLog, EventLog *newLog)
{
  EventLog l;
  int      event;
  int      ierr;

  PetscFunctionBegin;
  ierr = PetscNew(struct _EventLog, &l);                                                                  CHKERRQ(ierr);
  l->numEvents   = eventLog->numEvents;
  l->maxEvents   = eventLog->maxEvents;
  ierr = PetscMalloc(l->maxEvents * sizeof(PerfInfo), &l->eventInfo);                                     CHKERRQ(ierr);
  for(event = 0; event < eventLog->numEvents; event++) {
    ierr = PetscStrallocpy(eventLog->eventInfo[event].name, &l->eventInfo[event].name);                   CHKERRQ(ierr);
    l->eventInfo[event].id            = eventLog->eventInfo[event].id;
    l->eventInfo[event].cookie        = eventLog->eventInfo[event].cookie;
    l->eventInfo[event].active        = eventLog->eventInfo[event].active;
    l->eventInfo[event].visible       = eventLog->eventInfo[event].visible;
    l->eventInfo[event].depth         = 0;
    l->eventInfo[event].count         = 0;
    l->eventInfo[event].flops         = 0.0;
    l->eventInfo[event].time          = 0.0;
    l->eventInfo[event].numMessages   = 0.0;
    l->eventInfo[event].messageLength = 0.0;
    l->eventInfo[event].numReductions = 0.0;
#if defined(PETSC_HAVE_MPE)
    l->eventInfo[event].mpe_id_begin  = eventLog->eventInfo[event].mpe_id_begin;
    l->eventInfo[event].mpe_id_end    = eventLog->eventInfo[event].mpe_id_end;
#endif
  }
  *newLog = l;
  PetscFunctionReturn(0);
}

/*--------------------------------------------- Registration Functions ----------------------------------------------*/
#undef __FUNCT__  
#define __FUNCT__ "EventLogRegister"
/*@C
  EventLogRegister - Registers an event for logging operations in an application code.

  Not Collective

  Input Parameters:
+ eventLog     - The EventLog
. ename        - The name associated with the event
. cookie       - The cookie associated to the class for this event
. mpe_id_begin - Id indicating begin_event for MPE logging
- mpe_id_end   - Id indicating end_event for MPE logging

  Output Parameter:
. event    - The event

  Example of Usage:
.vb
      int USER_EVENT;
      int user_event_flops;
      PetscLogEventRegister(&USER_EVENT,"User event name");
      PetscLogEventBegin(USER_EVENT,0,0,0,0);
         [code segment to monitor]
         PetscLogFlops(user_event_flops);
      PetscLogEventEnd(USER_EVENT,0,0,0,0);
.ve

  Notes: 
  PETSc automatically logs library events if the code has been
  compiled with -DPETSC_USE_LOG (which is the default) and -log,
  -log_summary, or -log_all are specified.  PetscLogEventRegister() is
  intended for logging user events to supplement this PETSc
  information. 

  PETSc can gather data for use with the utilities Upshot/Nupshot
  (part of the MPICH distribution).  If PETSc has been compiled
  with flag -DPETSC_HAVE_MPE (MPE is an additional utility within
  MPICH), the user can employ another command line option, -log_mpe,
  to create a logfile, "mpe.log", which can be visualized
  Upshot/Nupshot.

  Level: intermediate

.keywords: log, event, register
.seealso: PetscLogEventBegin(), PetscLogEventEnd(), PetscLogFlops(), PetscLogEventMPEActivate(), PetscLogEventMPEDeactivate(),
          EventLogActivate(), EventLogDeactivate()
@*/
int EventLogRegister(EventLog eventLog, const char ename[], int cookie, int mpe_id_begin, int mpe_id_end, PetscEvent *event) {
  PerfInfo *eventInfo;
  char     *str;
  int       e;
  int       ierr;

  PetscFunctionBegin;
  PetscValidCharPointer(ename);
  PetscValidIntPointer(event);
  /* Should check cookie I think */
  e = eventLog->numEvents++;
  if (eventLog->numEvents > eventLog->maxEvents) {
    ierr = PetscMalloc(eventLog->maxEvents*2 * sizeof(PerfInfo), &eventInfo);                             CHKERRQ(ierr);
    ierr = PetscMemcpy(eventInfo,   eventLog->eventInfo,   eventLog->maxEvents * sizeof(PerfInfo));       CHKERRQ(ierr);
    ierr = PetscFree(eventLog->eventInfo);                                                                CHKERRQ(ierr);
    eventLog->eventInfo  = eventInfo;
    eventLog->maxEvents *= 2;
  }
  ierr = PetscStrallocpy(ename, &str);                                                                    CHKERRQ(ierr);
  eventLog->eventInfo[e].name          = str;
  eventLog->eventInfo[e].cookie        = cookie;
  eventLog->eventInfo[e].active        = PETSC_TRUE;
  eventLog->eventInfo[e].visible       = PETSC_TRUE;
  eventLog->eventInfo[e].depth         = 0;
  eventLog->eventInfo[e].count         = 0;
  eventLog->eventInfo[e].flops         = 0.0;
  eventLog->eventInfo[e].time          = 0.0;
  eventLog->eventInfo[e].numMessages   = 0.0;
  eventLog->eventInfo[e].messageLength = 0.0;
  eventLog->eventInfo[e].numReductions = 0.0;
  eventLog->eventInfo[e].id            = e;
#if defined(PETSC_HAVE_MPE)
  eventLog->eventInfo[e].mpe_id_begin = mpe_id_begin;
  eventLog->eventInfo[e].mpe_id_end   = mpe_id_end;
#endif
  *event = eventLog->eventInfo[e].id;
  PetscFunctionReturn(0);
}


/*---------------------------------------------- Activation Functions -----------------------------------------------*/
#undef __FUNCT__  
#define __FUNCT__ "EventLogActivate"
/*@C
  EventLogActivate - Indicates that a particular event should be logged.

  Not Collective

  Input Parameters:
+ eventLog - The EventLog
- event    - The event

   Usage:
.vb
      EventLogDeactivate(log, VEC_SetValues);
        [code where you do not want to log VecSetValues()]
      EventLogActivate(log, VEC_SetValues);
        [code where you do want to log VecSetValues()]
.ve 

  Note:
  The event may be either a pre-defined PETSc event (found in 
  include/petsclog.h) or an event number obtained with EventLogRegister().

  Level: advanced

.keywords: log, event, activate
.seealso: PetscLogEventMPEDeactivate(), PetscLogEventMPEActivate(), EventLogDeactivate()
@*/
int EventLogActivate(EventLog eventLog, PetscEvent event) {
  PetscFunctionBegin;
  eventLog->eventInfo[event].active = PETSC_TRUE;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "EventLogDeactivate"
/*@C
  EventLogDeactivate - Indicates that a particular event should not be logged.

  Not Collective

  Input Parameters:
+ eventLog - The EventLog
- event    - The event

   Usage:
.vb
      EventLogDeactivate(log, VEC_SetValues);
        [code where you do not want to log VecSetValues()]
      EventLogActivate(log, VEC_SetValues);
        [code where you do want to log VecSetValues()]
.ve 

  Note:
  The event may be either a pre-defined PETSc event (found in 
  include/petsclog.h) or an event number obtained with EventLogRegister().

  Level: advanced

.keywords: log, event, activate
.seealso: PetscLogEventMPEDeactivate(), PetscLogEventMPEActivate(), EventLogActivate()
@*/
int EventLogDeactivate(EventLog eventLog, PetscEvent event) {
  PetscFunctionBegin;
  eventLog->eventInfo[event].active = PETSC_FALSE;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "EventLogActivateClass"
/*@C
  EventLogActivateClass - Activates event logging for a PETSc object class.

  Not Collective

  Input Parameter:
. cookie - The class id, for example MAT_COOKIE, SNES_COOKIE,

  Level: developer

.seealso: EventLogDeactivateClass(), EventLogActivate(), EventLogDeactivate()
@*/
int EventLogActivateClass(EventLog eventLog, int cookie)
{
  int e;

  PetscFunctionBegin;
  for(e = 0; e < eventLog->numEvents; e++) {
    if (eventLog->eventInfo[e].cookie == cookie) eventLog->eventInfo[e].active = PETSC_TRUE;
  }
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "EventLogDeactivateClass"
/*@C
  EventLogDeactivateClass - Deactivates event logging for a PETSc object class.

  Not Collective

  Input Parameter:
. cookie - The class id, for example MAT_COOKIE, SNES_COOKIE,

  Level: developer

.seealso: EventLogDeactivateClass(), EventLogDeactivate(), EventLogActivate()
@*/
int EventLogDeactivateClass(EventLog eventLog, int cookie)
{
  int e;

  PetscFunctionBegin;
  for(e = 0; e < eventLog->numEvents; e++) {
    if (eventLog->eventInfo[e].cookie == cookie) eventLog->eventInfo[e].active = PETSC_FALSE;
  }
  PetscFunctionReturn(0);
}

/*------------------------------------------------ Query Functions --------------------------------------------------*/
#undef __FUNCT__  
#define __FUNCT__ "EventLogSetVisible"
/*@C
  EventLogSetVisible - This function determines whether an event is printed during PetscLogPrintSummary()

  Not Collective

  Input Parameters:
+ eventLog  - The EventLog
. event     - The event to log
- isVisible - The visibility flag, PETSC_TRUE for printing, otherwise PETSC_FALSE (default is PETSC_TRUE)

  Database Options:
. -log_summary - Activates log summary

  Level: intermediate

.keywords: log, visible, event
.seealso: EventLogGetVisible(), EventLogGetCurrent(), EventLogRegister(), StageLogGetEventLog()
@*/
int EventLogSetVisible(EventLog eventLog, PetscEvent event, PetscTruth isVisible) {
  PetscFunctionBegin;
  eventLog->eventInfo[event].visible = isVisible;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "EventLogGetVisible"
/*@C
  EventLogGetVisible - This function returns whether an event is printed during PetscLogPrintSummary()

  Not Collective

  Input Parameters:
+ eventLog  - The EventLog
- event     - The event id to log

  Output Parameter:
. isVisible - The visibility flag, PETSC_TRUE for printing, otherwise PETSC_FALSE (default is PETSC_TRUE)

  Database Options:
. -log_summary - Activates log summary

  Level: intermediate

.keywords: log, visible, event
.seealso: EventLogSetVisible(), EventLogGetCurrent(), EventLogRegister(), StageLogGetEventLog()
@*/
int EventLogGetVisible(EventLog eventLog, PetscEvent event, PetscTruth *isVisible) {
  PetscFunctionBegin;
  PetscValidIntPointer(isVisible);
  *isVisible = eventLog->eventInfo[event].visible;
  PetscFunctionReturn(0);
}

/*------------------------------------------------ Action Functions -------------------------------------------------*/
#undef __FUNCT__  
#define __FUNCT__ "PetscLogEventBeginDefault"
int PetscLogEventBeginDefault(PetscEvent event, int t, PetscObject o1, PetscObject o2, PetscObject o3, PetscObject o4) {
  StageLog stageLog;
  EventLog eventLog;
  int      stage;
  int      ierr;

  PetscFunctionBegin;
  ierr = PetscLogGetStageLog(&stageLog);                                                                  CHKERRQ(ierr);
  ierr = StageLogGetCurrent(stageLog, &stage);                                                            CHKERRQ(ierr);
  ierr = StageLogGetEventLog(stageLog, stage, &eventLog);                                                 CHKERRQ(ierr);
  /* Check for double counting */
  eventLog->eventInfo[event].depth++;
  if (eventLog->eventInfo[event].depth > 1) PetscFunctionReturn(0);
  /* Log performance info */
  eventLog->eventInfo[event].count++;
  PetscTimeSubtract(eventLog->eventInfo[event].time);
  eventLog->eventInfo[event].flops         -= _TotalFlops;
  eventLog->eventInfo[event].numMessages   -= irecv_ct  + isend_ct  + recv_ct  + send_ct;
  eventLog->eventInfo[event].messageLength -= irecv_len + isend_len + recv_len + send_len;
  eventLog->eventInfo[event].numReductions -= allreduce_ct;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "PetscLogEventEndDefault"
int PetscLogEventEndDefault(PetscEvent event, int t, PetscObject o1, PetscObject o2, PetscObject o3, PetscObject o4) {
  StageLog stageLog;
  EventLog eventLog;
  int      stage;
  int      ierr;

  PetscFunctionBegin;
  ierr = PetscLogGetStageLog(&stageLog);                                                                  CHKERRQ(ierr);
  ierr = StageLogGetCurrent(stageLog, &stage);                                                            CHKERRQ(ierr);
  ierr = StageLogGetEventLog(stageLog, stage, &eventLog);                                                 CHKERRQ(ierr);
  /* Check for double counting */
  eventLog->eventInfo[event].depth--;
  if (eventLog->eventInfo[event].depth > 0) {
    PetscFunctionReturn(0);
  } else if (eventLog->eventInfo[event].depth < 0) {
    SETERRQ(PETSC_ERR_ARG_WRONGSTATE, "Logging event had unbalanced begin/end pairs");
  }
  /* Log performance info */
  PetscTimeAdd(eventLog->eventInfo[event].time);
  eventLog->eventInfo[event].flops         += _TotalFlops;
  eventLog->eventInfo[event].numMessages   += irecv_ct  + isend_ct  + recv_ct  + send_ct;
  eventLog->eventInfo[event].messageLength += irecv_len + isend_len + recv_len + send_len;
  eventLog->eventInfo[event].numReductions += allreduce_ct;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "PetscLogEventBeginComplete"
int PetscLogEventBeginComplete(PetscEvent event, int t, PetscObject o1, PetscObject o2, PetscObject o3, PetscObject o4) {
  StageLog       stageLog;
  EventLog       eventLog;
  Action        *tmpAction;
  PetscLogDouble start, end;
  PetscLogDouble curTime;
  int            stage;
  int            ierr;

  PetscFunctionBegin;
  /* Dynamically enlarge logging structures */
  if (numActions >= maxActions) {
    PetscTime(start);
    ierr = PetscMalloc(maxActions*2 * sizeof(Action), &tmpAction);                                        CHKERRQ(ierr);
    ierr = PetscMemcpy(tmpAction, actions, maxActions * sizeof(Action));                                  CHKERRQ(ierr);
    ierr = PetscFree(actions);                                                                            CHKERRQ(ierr);
    actions     = tmpAction;
    maxActions *= 2;
    PetscTime(end);
    BaseTime += (end - start);
  }
  /* Record the event */
  ierr = PetscLogGetStageLog(&stageLog);                                                                  CHKERRQ(ierr);
  ierr = StageLogGetCurrent(stageLog, &stage);                                                            CHKERRQ(ierr);
  ierr = StageLogGetEventLog(stageLog, stage, &eventLog);                                                 CHKERRQ(ierr);
  PetscTime(curTime);
  if (actions != PETSC_NULL) {
    actions[numActions].time     = curTime - BaseTime;
    actions[numActions++].action = ACTIONBEGIN;
    actions[numActions].event    = event;
    actions[numActions].cookie   = eventLog->eventInfo[event].cookie;
    if (o1) actions[numActions].id1 = o1->id; else actions[numActions].id1 = -1;
    if (o2) actions[numActions].id2 = o2->id; else actions[numActions].id2 = -1;
    if (o3) actions[numActions].id3 = o3->id; else actions[numActions].id3 = -1;
    actions[numActions].flops    = _TotalFlops;
    ierr = PetscTrSpace(&actions[numActions].mem, PETSC_NULL, &actions[numActions].maxmem);               CHKERRQ(ierr);
  }
  /* Check for double counting */
  eventLog->eventInfo[event].depth++;
  if (eventLog->eventInfo[event].depth > 1) PetscFunctionReturn(0);
  /* Log the performance info */
  eventLog->eventInfo[event].count++;
  eventLog->eventInfo[event].time          -= curTime;
  eventLog->eventInfo[event].flops         -= _TotalFlops;
  eventLog->eventInfo[event].numMessages   -= irecv_ct  + isend_ct  + recv_ct  + send_ct;
  eventLog->eventInfo[event].messageLength -= irecv_len + isend_len + recv_len + send_len;
  eventLog->eventInfo[event].numReductions -= allreduce_ct;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "PetscLogEventEndComplete"
int PetscLogEventEndComplete(PetscEvent event, int t, PetscObject o1, PetscObject o2, PetscObject o3, PetscObject o4) {
  StageLog       stageLog;
  EventLog       eventLog;
  Action        *tmpAction;
  PetscLogDouble start, end;
  PetscLogDouble curTime;
  int            stage;
  int            ierr;

  PetscFunctionBegin;
  /* Dynamically enlarge logging structures */
  if (numActions >= maxActions) {
    PetscTime(start);
    ierr = PetscMalloc(maxActions*2 * sizeof(Action), &tmpAction);                                        CHKERRQ(ierr);
    ierr = PetscMemcpy(tmpAction, actions, maxActions * sizeof(Action));                                  CHKERRQ(ierr);
    ierr = PetscFree(actions);                                                                            CHKERRQ(ierr);
    actions     = tmpAction;
    maxActions *= 2;
    PetscTime(end);
    BaseTime += (end - start);
  }
  /* Record the event */
  ierr = PetscLogGetStageLog(&stageLog);                                                                  CHKERRQ(ierr);
  ierr = StageLogGetCurrent(stageLog, &stage);                                                            CHKERRQ(ierr);
  ierr = StageLogGetEventLog(stageLog, stage, &eventLog);                                                 CHKERRQ(ierr);
  PetscTime(curTime);
  if (actions != PETSC_NULL) {
    actions[numActions].time     = curTime - BaseTime;
    actions[numActions++].action = ACTIONEND;
    actions[numActions].event    = event;
    actions[numActions].cookie   = eventLog->eventInfo[event].cookie;
    if (o1) actions[numActions].id1 = o1->id; else actions[numActions].id1 = -1;
    if (o2) actions[numActions].id2 = o2->id; else actions[numActions].id2 = -1;
    if (o3) actions[numActions].id3 = o3->id; else actions[numActions].id3 = -1;
    actions[numActions].flops    = _TotalFlops;
    ierr = PetscTrSpace(&actions[numActions].mem, PETSC_NULL, &actions[numActions].maxmem);               CHKERRQ(ierr);
  }
  /* Check for double counting */
  eventLog->eventInfo[event].depth--;
  if (eventLog->eventInfo[event].depth > 0) {
    PetscFunctionReturn(0);
  } else if (eventLog->eventInfo[event].depth < 0) {
    SETERRQ(PETSC_ERR_ARG_WRONGSTATE, "Logging event had unbalanced begin/end pairs");
  }
  /* Log the performance info */
  eventLog->eventInfo[event].count++;
  eventLog->eventInfo[event].time          += curTime;
  eventLog->eventInfo[event].flops         += _TotalFlops;
  eventLog->eventInfo[event].numMessages   += irecv_ct  + isend_ct  + recv_ct  + send_ct;
  eventLog->eventInfo[event].messageLength += irecv_len + isend_len + recv_len + send_len;
  eventLog->eventInfo[event].numReductions += allreduce_ct;
  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "PetscLogEventBeginTrace"
int PetscLogEventBeginTrace(PetscEvent event, int t, PetscObject o1, PetscObject o2, PetscObject o3, PetscObject o4) {
  StageLog       stageLog;
  EventLog       eventLog;
  PetscLogDouble cur_time;
  int            rank, stage;
  int            ierr;

  PetscFunctionBegin;
  if (tracetime == 0.0) {PetscTime(tracetime);}

  ierr = MPI_Comm_rank(PETSC_COMM_WORLD, &rank);                                                          CHKERRQ(ierr);
  ierr = PetscLogGetStageLog(&stageLog);                                                                  CHKERRQ(ierr);
  ierr = StageLogGetCurrent(stageLog, &stage);                                                            CHKERRQ(ierr);
  ierr = StageLogGetEventLog(stageLog, stage, &eventLog);                                                 CHKERRQ(ierr);
  /* Check for double counting */
  eventLog->eventInfo[event].depth++;
  if (eventLog->eventInfo[event].depth > 1) PetscFunctionReturn(0);
  /* Log performance info */
  ierr = PetscStrncpy(tracespace, traceblanks, 2*tracelevel);                                             CHKERRQ(ierr);
  tracespace[2*tracelevel] = 0;
  PetscTime(cur_time);
  fprintf(tracefile, "%s[%d] %g Event begin: %s\n", tracespace, rank, cur_time-tracetime, eventLog->eventInfo[event].name);
  fflush(tracefile);
  tracelevel++;

  PetscFunctionReturn(0);
}

#undef __FUNCT__  
#define __FUNCT__ "PetscLogEventEndTrace"
int PetscLogEventEndTrace(PetscEvent event,int t,PetscObject o1,PetscObject o2,PetscObject o3,PetscObject o4) {
  StageLog       stageLog;
  EventLog       eventLog;
  PetscLogDouble cur_time;
  int            rank, stage;
  int            ierr;

  PetscFunctionBegin;
  tracelevel--;
  ierr = MPI_Comm_rank(PETSC_COMM_WORLD, &rank);                                                          CHKERRQ(ierr);
  ierr = PetscLogGetStageLog(&stageLog);                                                                  CHKERRQ(ierr);
  ierr = StageLogGetCurrent(stageLog, &stage);                                                            CHKERRQ(ierr);
  ierr = StageLogGetEventLog(stageLog, stage, &eventLog);                                                 CHKERRQ(ierr);
  /* Check for double counting */
  eventLog->eventInfo[event].depth--;
  if (eventLog->eventInfo[event].depth > 0) {
    PetscFunctionReturn(0);
  } else if (eventLog->eventInfo[event].depth < 0) {
    SETERRQ(PETSC_ERR_ARG_WRONGSTATE, "Logging event had unbalanced begin/end pairs");
  }
  /* Log performance info */
  ierr = PetscStrncpy(tracespace, traceblanks, 2*tracelevel);                                             CHKERRQ(ierr);
  tracespace[2*tracelevel] = 0;
  PetscTime(cur_time);
  fprintf(tracefile, "%s[%d] %g Event end: %s\n", tracespace, rank, cur_time-tracetime, eventLog->eventInfo[event].name);
  fflush(tracefile);
  PetscFunctionReturn(0);
}

/*----------------------------------------------- Creation Function -------------------------------------------------*/
#undef __FUNCT__  
#define __FUNCT__ "EventLogCreate"
/*
  EventLogCreate - This creates a EventLog object.

  Not collective

  Input Parameter:
. eventLog - The EventLog

  Level: beginner

.keywords: log, event, create
.seealso: EventLogDestroy(), StageLogCreate()
*/
int EventLogCreate(EventLog *eventLog) {
  EventLog l;
  int      ierr;

  PetscFunctionBegin;
  ierr = PetscNew(struct _EventLog, &l);                                                                  CHKERRQ(ierr);
  l->numEvents   = 0;
  l->maxEvents   = 100;
  ierr = PetscMalloc(l->maxEvents * sizeof(PerfInfo), &l->eventInfo);                                     CHKERRQ(ierr);
  *eventLog = l;
  PetscFunctionReturn(0);
}
