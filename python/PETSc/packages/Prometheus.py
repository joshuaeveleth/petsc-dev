#!/usr/bin/env python
from __future__ import generators
import user
import config.base
import os
import PETSc.package

class Configure(PETSc.package.Package):
  def __init__(self, framework):
    PETSc.package.Package.__init__(self, framework)
    self.mpi               = self.framework.require('PETSc.packages.MPI',self)
    self.blasLapack        = self.framework.require('PETSc.packages.BlasLapack',self)
    self.parmetis          = self.framework.require('PETSc.packages.ParMetis',self)
    self.download          = ['ftp://ftp.mcs.anl.gov/pub/petsc/externalpackages/Prometheus-1.8.0-patch.tar.gz']
    self.deps              = [self.parmetis,self.mpi,self.blasLapack]
    self.functions         = None
    self.includes          = None
    self.liblist           = [['libpromfei.a','libprometheus.a']]
    self.cxx               = 1   # requires C++
    self.compilePrometheus = 0
    return

  def Install(self):
    prometheusDir = self.getDir()
    installDir = os.path.join(prometheusDir, self.arch.arch)
    if not os.path.isdir(os.path.join(installDir,'lib')):
      os.mkdir(os.path.join(installDir,'lib'))
      os.mkdir(os.path.join(installDir,'include'))            
    self.framework.pushLanguage('C')
    args  = 'C_CC = '+self.framework.getCompiler()+'\n'
    args += 'PETSC_INCLUDE = -I'+os.path.join(self.framework.argDB['PETSC_DIR'],'bmake',self.arch.arch)+' -I'+os.path.join(self.framework.argDB['PETSC_DIR'])+' -I'+os.path.join(self.framework.argDB['PETSC_DIR'],'include')+' '+' '.join([self.libraries.getIncludeArgument(inc) for inc in self.mpi.include+self.parmetis.include])+'\n'
    args += 'BUILD_DIR  = '+prometheusDir+'\n'
    args += 'LIB_DIR  = $(BUILD_DIR)/lib/\n'
    args += 'RANLIB = '+self.setcompilers.RANLIB+'\n'
    self.framework.popLanguage()
    self.framework.pushLanguage('C++')+'\n'
    args += 'CC = '+self.framework.getCompiler()+'\n'    
    args += 'PETSCFLAGS = '+self.framework.getCompilerFlags()+'\n'
    self.framework.popLanguage()
    try:
      fd      = file(os.path.join(installDir,'makefile.in'))
      oldargs = fd.readline()
      fd.close()
    except:
      oldargs = ''
    if not oldargs == args:
      self.framework.log.write('Have to rebuild Prometheus oldargs = '+oldargs+' new args '+args+'\n')
      self.logPrintBox('Configuring Prometheus; this may take a minute')
      fd = file(os.path.join(installDir,'makefile.in'),'w')
      fd.write(args)
      fd.close()
      fd = file(os.path.join(prometheusDir,'makefile.in'),'w')
      fd.write(args)
      fd.close()
      self.compilePrometheus = 1
      self.prometheusDir     = prometheusDir
      self.installDir        = installDir
    return prometheusDir

  def postProcess(self):
    if self.compilePrometheus:
      self.logPrintBox('Compiling Prometheus; this may take several minutes')
      output  = config.base.Configure.executeShellCommand('cd '+self.prometheusDir+'; Make prometheus; mv '+os.path.join('lib','lib*.a')+' '+os.path.join(self.installDir,'lib'),timeout=250, log = self.framework.log)[0]
      self.framework.log.write(output)
      output  = config.base.Configure.executeShellCommand('cp '+os.path.join(self.prometheusDir,'include','*.*')+' '+os.path.join(self.prometheusDir,'fei_prom','*.h')+' '+os.path.join(self.installDir,'include'),timeout=250, log = self.framework.log)[0]      
      self.framework.log.write(output)
      try:
        output  = config.base.Configure.executeShellCommand(self.setcompilers.RANLIB+' '+os.path.join(self.installDir,'lib')+'/libprometheus.a', timeout=250, log = self.framework.log)[0]
        self.framework.log.write(output)
      except RuntimeError, e:
        raise RuntimeError('Error running ranlib on PROMETHEUS libraries: '+str(e))

if __name__ == '__main__':
  import config.framework
  import sys
  framework = config.framework.Framework(sys.argv[1:])
  framework.setupLogging(framework.clArgs)
  framework.children.append(Configure(framework))
  framework.configure()
  framework.dumpSubstitutions()
