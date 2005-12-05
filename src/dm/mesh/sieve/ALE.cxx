#define ALE_ALE_cxx

#include <ALE.hh>

namespace ALE {

  //
  //  Memory handling stuff (ALE_mem.hh).
  // 
  
  // static instance of a standard char allocator;  this is the only allocator used by ALE; 
  // it is defined here -- in an .cxx file -- to ensure that exactly one copy exists;
  // its services are presented through a static interface defined in universal_allocator.

  std::allocator<char> _alloc;

  char *universal_allocator::allocate(const universal_allocator::size_type& sz) {
    return _alloc.allocate(sz);
  }

  void universal_allocator::deallocate(char *p, const universal_allocator::size_type& sz) {
    return _alloc.deallocate(p,sz);
  }

  universal_allocator::size_type universal_allocator::max_size() {
    return _alloc.max_size();
  }

  //
  // Error/exception handling helper functions (ALE_exception.hh).
  //

  // A helper function that throws an ALE::Exception with a message identifying the function that returned the given error code, 
  // including the function and the line where the error occured.
  void ERROR(PetscErrorCode ierr, const char *func, int line, const char *msg) {
    if(ierr) {
        int32_t buf_size = 2*1024;
        char *mess = (char *)malloc(sizeof(char)*(buf_size+1));
        snprintf(mess, buf_size, "%s: line %d: error %d: %s:\n", func, line, (int)ierr, msg);
        throw ALE::Exception(mess);
    }
  }// ERROR()

  const char *ERRORMSG(const char *fmt, ...);

  // A helper function for converting MPI errors to exception
  void MPIERROR(PetscErrorCode ierr, const char *func, int line, const char *msg) {
    if(ierr) {
      char mpi_error[MPI_MAX_ERROR_STRING+1];
      int32_t len = MPI_MAX_ERROR_STRING;
      PetscErrorCode ie = MPI_Error_string(ierr, mpi_error, &len);
      char *mess;
      if(!ie) {
        mess = (char *)malloc(sizeof(char)*(strlen(msg)+len+1));
        sprintf(mess, "%s: %s", msg, mpi_error);
      }
      else {
        mess = (char *)malloc(sizeof(char)*(strlen(msg)));
        sprintf(mess, "%s: <unknown error>", msg);
      }
      ERROR(ierr, func, line, mess);
    }
  }// MPIERROR()

  // A helper function that allocates and assembles an error message from a format string 
  const char *ERRORMSG(const char *fmt, ...) {
    va_list Argp;
    int32_t buf_size = 2*MPI_MAX_ERROR_STRING;
    if(fmt) {
      va_start(Argp, fmt);
      char *msg = (char *)malloc(sizeof(char)*(buf_size+1));
      snprintf(msg, buf_size, fmt, Argp);
      va_end(Argp);
      return msg;
    }
    return fmt;
  }// ERRORMSG()

  //
  // Logging helper functions
  //

  static std::map<std::string, int> _log_stage;  // a map from stage names to stage numbers

  #undef  __FUNCT__
  #define __FUNCT__ "LogCookieRegister"
  LogCookie LogCookieRegister(const char *name){
    LogCookie cookie;
    PetscErrorCode ierr = PetscLogClassRegister(&cookie, name);
    CHKERROR(ierr, "PetscLogClassRegister failed");
    return cookie;
  }

  #undef  __FUNCT__
  #define __FUNCT__ "LogStageRegister"
  int LogStageRegister(const char *name){
    int stage;
    std::string stage_name(name);
    if(_log_stage.find(stage_name) == _log_stage.end()) {    
      // stage by that name not yet registered, so we register it and store its registration number.
      PetscErrorCode ierr = PetscLogStageRegister(&stage, name); CHKERROR(ierr, "PetscLogStageRegister failed");
      _log_stage[stage_name] = stage;                   
    }                                                        
    else {                                                   
      // stage by that name already registered, so we retrieve its registration number.
      stage = _log_stage[stage_name];                   
    }                                                        
    return stage;
  }

  #undef  __FUNCT__
  #define __FUNCT__ "LogStagePush"
  void LogStagePush(int s){
    PetscErrorCode ierr;
    ierr = PetscLogStagePush(s); CHKERROR(ierr, "PetscLogStagePush failed");
  }//LogStagePush()

  #undef  __FUNCT__
  #define __FUNCT__ "LogStagePop"
  void LogStagePop(int s){
    // A future implementation may use 's' to check for the correct order of stage push/pop events.
    PetscErrorCode ierr;
    ierr = PetscLogStagePop(); CHKERROR(ierr, "PetscLogStagePop failed");
  }//LogStagePop()

  #undef  __FUNCT__
  #define __FUNCT__ "LogEventRegister"
  LogEvent LogEventRegister(LogCookie cookie, const char *event_name){
    LogEvent event;
    PetscErrorCode ierr = PetscLogEventRegister(&event, event_name, cookie);
    CHKERROR(ierr, "PetscLogEventRegister failed");
    return event;
  }

  #undef  __FUNCT__
  #define __FUNCT__ "LogEventBegin"
  void LogEventBegin(int e){
    PetscErrorCode ierr;
    ierr = PetscLogEventBegin(e, 0, 0, 0, 0); CHKERROR(ierr, "PetscLogEventBegin failed");
  }//LogEventBegin()

  #undef  __FUNCT__
  #define __FUNCT__ "LogEventEnd"
  void LogEventEnd(int e){
    PetscErrorCode ierr;
    ierr = PetscLogEventEnd(e, 0, 0, 0, 0); CHKERROR(ierr, "PetscLogEventEnd failed");
  }//LogEventEnd()

}

#undef ALE_ALE_cxx
