/***************************************************************************
                        synchronizer.cpp  -  description
                             -------------------
    copyright            : (C) 2003 by Csaba Karai
    e-mail               : krusader@users.sourceforge.net
    web site             : http://krusader.sourceforge.net
 ---------------------------------------------------------------------------
  Description
 ***************************************************************************

  A

     db   dD d8888b. db    db .d8888.  .d8b.  d8888b. d88888b d8888b.
     88 ,8P' 88  `8D 88    88 88'  YP d8' `8b 88  `8D 88'     88  `8D
     88,8P   88oobY' 88    88 `8bo.   88ooo88 88   88 88ooooo 88oobY'
     88`8b   88`8b   88    88   `Y8b. 88~~~88 88   88 88~~~~~ 88`8b
     88 `88. 88 `88. 88b  d88 db   8D 88   88 88  .8D 88.     88 `88.
     YP   YD 88   YD ~Y8888P' `8888Y' YP   YP Y8888D' Y88888P 88   YD

                                                     S o u r c e    F i l e

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "synchronizer.h"
#include "../VFS/ftp_vfs.h"
#include "../VFS/normal_vfs.h"
#include "../VFS/vfile.h"
#include <kurl.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <qapplication.h>
#include <qregexp.h>
#include <kio/job.h>
#include <kdialogbase.h>
#include <kio/observer.h>
#include <kio/renamedlg.h>
#include <kio/skipdlg.h>
#include <unistd.h>

#define  DISPLAY_UPDATE_PERIOD        2

Synchronizer::Synchronizer() : displayUpdateCount( 0 ), markEquals( true ), markDiffers ( true ),
                               markCopyToLeft( true ), markCopyToRight( true ), markDeletable( true )
{
  resultList.setAutoDelete( true );
}

void Synchronizer::compare( QString leftURL, QString rightURL, QString filter, bool subDirs,
                            bool symLinks, bool igDate, bool asymm )
{
  resultList.clear();

  recurseSubDirs = subDirs;
  followSymLinks = symLinks;
  ignoreDate     = igDate;
  asymmetric     = asymm;
  fileFilter     = filter;
  stopped = false;

  if( !leftURL.endsWith("/" ))  leftURL+="/";
  if( !rightURL.endsWith("/" )) rightURL+="/";

  scannedDirs = fileCount = 0;
    
  compareDirectory( 0, leftBaseDir = leftURL, rightBaseDir = rightURL, "" );

  emit statusInfo( i18n( "File number:%1" ).arg( fileCount ) );
}

void Synchronizer::compareDirectory( SynchronizerFileItem *parent, QString leftURL,
                                     QString rightURL, QString dir, QString addName,
                                     QString addDir, time_t addLTime, time_t addRTime )
{
  vfs   * left_directory  = getDirectory( leftURL );
  vfs   * right_directory = getDirectory( rightURL );
  vfile * left_file;
  vfile * right_file;
  QString file_name;

  if( left_directory == 0 || right_directory == 0 )
  {
    if( left_directory )
      delete left_directory;
    if( right_directory )
      delete right_directory;    
    return;
  }

  if( !dir.isEmpty() )
    parent = addDuplicateItem( parent, addName, addDir, 0, 0, addLTime, addRTime, true );

  /* walking through in the left directory */  
  for( left_file=left_directory->vfs_getFirstFile(); left_file != 0 ;
       left_file=left_directory->vfs_getNextFile() )
  {
    if ( left_file->vfile_isDir() || left_file->vfile_isSymLink() )
      continue;

    file_name =  left_file->vfile_getName();

    if( !checkName( file_name ) )
      continue;

    if( (right_file = right_directory->vfs_search( file_name )) == 0 )
      addLeftOnlyItem( parent, file_name, dir, left_file->vfile_getSize(), left_file->vfile_getTime_t() );
    else
    {
      if( right_file->vfile_isDir() || right_file->vfile_isSymLink() )
        continue;

      addDuplicateItem( parent, file_name, dir, left_file->vfile_getSize(), right_file->vfile_getSize(),
                        left_file->vfile_getTime_t(), right_file->vfile_getTime_t() );
    }
  }

  /* walking through in the right directory */
  for( right_file=right_directory->vfs_getFirstFile(); right_file != 0 ;
       right_file=right_directory->vfs_getNextFile() )
  {
    if( right_file->vfile_isDir() || right_file->vfile_isSymLink() )
      continue;

    file_name =  right_file->vfile_getName();

    if( !checkName( file_name ) )
      continue;
    
    if( left_directory->vfs_search( file_name ) == 0 )
      addRightOnlyItem( parent, file_name, dir, right_file->vfile_getSize(), right_file->vfile_getTime_t() );
  }

  /* walking through the subdirectories */
  if( recurseSubDirs )
  {
    for( left_file=left_directory->vfs_getFirstFile(); left_file != 0 ;
         left_file=left_directory->vfs_getNextFile() )
    {
      if ( left_file->vfile_isDir() && ( followSymLinks || !left_file->vfile_isSymLink()) )
      {
        file_name =  left_file->vfile_getName();
        
        if( (right_file = right_directory->vfs_search( file_name )) == 0 )
          addSingleDirectory( parent, file_name, dir, left_file->vfile_getTime_t(), true );
        else
          compareDirectory( parent, leftURL+file_name+"/", rightURL+file_name+"/",
                            dir.isEmpty() ? file_name : dir+"/"+file_name, file_name,
                            dir, left_file->vfile_getTime_t(), right_file->vfile_getTime_t() );
      }
    }

    for( right_file=right_directory->vfs_getFirstFile(); right_file != 0 ;
         right_file=right_directory->vfs_getNextFile() )
    {
      if ( right_file->vfile_isDir() && (followSymLinks || !right_file->vfile_isSymLink()) )
      {
        file_name =  right_file->vfile_getName();

        if( left_directory->vfs_search( file_name ) == 0 )
          addSingleDirectory( parent, file_name, dir, right_file->vfile_getTime_t(), false );
      }
    }
  }
     
  delete left_directory;
  delete right_directory;
}

vfs * Synchronizer::getDirectory( QString url )
{
  if( stopped )
    return 0;
    
  vfs *v;
  bool isNormalVFS;
  
  if( url.startsWith( "/" ) || url.startsWith( "file:/" ))   /* is it normal vfs? */
  {
    v = new normal_vfs(url,0);
    isNormalVFS = true;
  }
  else                          /* ftp vfs */
  {
    v = new ftp_vfs(url,0);
    isNormalVFS = false;
  }

  if ( v->vfs_error() )
  {
    KMessageBox::error(0, i18n("Error at opening URL:%1!").arg( url ));
    delete v;
    return 0;
  }

  bool result = true;
  
  if( isNormalVFS )
    result = v->vfs_refresh( url );
  else
  {
    while( ((ftp_vfs *)v)->isBusy() )
    {
      qApp->processEvents();
      usleep( 100 );
    }
  }

  if ( !result || v->vfs_error() )
  {
    KMessageBox::error(0, i18n("Error at opening URL:%1!").arg( url ));
    delete v;
    return 0;
  }

  emit statusInfo( i18n( "Scanned directories:%1" ).arg( scannedDirs++ ) );
  
  return v;
}

QString Synchronizer::getTaskTypeName( TaskType taskType )
{
  static QString names[] = {"=","!=","<-","->","DEL"};

  return names[taskType];
}

SynchronizerFileItem * Synchronizer::addItem( SynchronizerFileItem *parent, QString file_name,
                                  QString dir, bool existsLeft, bool existsRight,
                                  KIO::filesize_t leftSize, KIO::filesize_t rightSize,
                                  time_t leftDate, time_t rightDate, TaskType tsk, bool isDir )
{
  bool marked = isMarked( tsk, existsLeft && existsRight );
  SynchronizerFileItem *item = new SynchronizerFileItem( file_name, dir, marked, 
    existsLeft, existsRight, leftSize, rightSize, leftDate, rightDate, tsk, isDir, parent );

  resultList.append( item );

  if( marked )
  {
    markParentDirectories( item );
    fileCount++;
    emit comparedFileData( item );
  }

  if( displayUpdateCount++ % DISPLAY_UPDATE_PERIOD == (DISPLAY_UPDATE_PERIOD-1) )
    qApp->processEvents();
    
  return item;
}

SynchronizerFileItem * Synchronizer::addLeftOnlyItem( SynchronizerFileItem *parent, QString file_name,
                                    QString dir, KIO::filesize_t size, time_t date, bool isDir )
{
  return addItem( parent, file_name, dir, true, false, size, 0, date, 0,
                  asymmetric ? TT_DELETE : TT_COPY_TO_RIGHT, isDir );
}

SynchronizerFileItem * Synchronizer::addRightOnlyItem( SynchronizerFileItem *parent, QString file_name,
                                    QString dir, KIO::filesize_t size, time_t date, bool isDir )
{
  return addItem( parent, file_name, dir, false, true, 0, size, 0, date, TT_COPY_TO_LEFT, isDir );
}

SynchronizerFileItem * Synchronizer::addDuplicateItem( SynchronizerFileItem *parent, QString file_name,
                                     QString dir, KIO::filesize_t leftSize,
                                     KIO::filesize_t rightSize, time_t leftDate, time_t rightDate,
                                     bool isDir )
{
  TaskType        task;

  if( isDir || ( leftSize == rightSize && (ignoreDate || leftDate == rightDate) ) )
    task = TT_EQUALS;
  else
  {
    if( asymmetric )
      task = TT_COPY_TO_LEFT;
    else if( ignoreDate )
      task = TT_DIFFERS;
    else if( leftDate > rightDate )
      task = TT_COPY_TO_RIGHT;
    else if( leftDate < rightDate )
      task = TT_COPY_TO_LEFT;
    else
      task = TT_DIFFERS;  /* TODO TODO TODO TODO */
  }

  return addItem( parent, file_name, dir, true, true, leftSize, rightSize, leftDate, rightDate, task, isDir );
}

void Synchronizer::addSingleDirectory( SynchronizerFileItem *parent, QString name,
                                       QString dir , time_t date, bool isLeft )
{
  QString baseDir = (isLeft ? leftBaseDir : rightBaseDir );
  QString dirName = dir.isEmpty() ? name : dir+"/"+name;
  
  vfs   * directory  = getDirectory( baseDir+ dirName );
  vfile * file;
  QString file_name;

  if( directory == 0 )
    return;

  if( isLeft )
    parent = addLeftOnlyItem( parent, name, dir, 0, date, true );
  else
    parent = addRightOnlyItem( parent, name, dir, 0, date, true );

  /* walking through the directory files */
  for( file=directory->vfs_getFirstFile(); file != 0 ; file = directory->vfs_getNextFile() )
  {
    if ( file->vfile_isDir() || file->vfile_isSymLink() )
      continue;

    file_name =  file->vfile_getName();

    if( !checkName( file_name ) )
      continue;

    if( isLeft )
      addLeftOnlyItem( parent, file_name, dirName, file->vfile_getSize(), file->vfile_getTime_t() );
    else
      addRightOnlyItem( parent, file_name, dirName, file->vfile_getSize(), file->vfile_getTime_t() );
  }

  /* walking through the subdirectories */
  for( file=directory->vfs_getFirstFile(); file != 0 ; file=directory->vfs_getNextFile() )
  {
    if ( file->vfile_isDir() && (followSymLinks || !file->vfile_isSymLink())  )
    {
        file_name =  file->vfile_getName();
        addSingleDirectory( parent, file_name, dirName, file->vfile_getTime_t(), isLeft );
    }
  }

  delete directory;
}

bool Synchronizer::checkName( QString name )
{
  return QRegExp(fileFilter,true,true).exactMatch( name );
}

void Synchronizer::setMarkFlags( bool left, bool equal, bool differs, bool right, bool dup, bool sing,
                                 bool del )
{
  markEquals      = equal;
  markDiffers     = differs;
  markCopyToLeft  = left;
  markCopyToRight = right;
  markDeletable   = del;
  markDuplicates  = dup;
  markSingles     = sing;
}

bool Synchronizer::isMarked( TaskType task, bool isDuplicate )
{
  if( (isDuplicate && !markDuplicates) || (!isDuplicate && !markSingles) )
    return false;
  
  switch( task )
  {
  case TT_EQUALS:
    return markEquals;
  case TT_DIFFERS:
    return markDiffers;
  case TT_COPY_TO_LEFT:
    return markCopyToLeft;
  case TT_COPY_TO_RIGHT:
    return markCopyToRight;
  case TT_DELETE:
    return markDeletable;
  }
  return false;
}

void Synchronizer::markParentDirectories( SynchronizerFileItem *item )
{
  if( item->parent() == 0 || item->parent()->isMarked() ) 
    return;

  markParentDirectories( item->parent() );
  item->parent()->setMarked( true );
  fileCount++;
  emit comparedFileData( item->parent() );
}

void Synchronizer::refresh()
{
  int step = 0;
  fileCount = 0;
  
  SynchronizerFileItem *item = resultList.first();

  while( item )
  {
    bool marked = isMarked( item->task(), item->existsInLeft() && item->existsInRight() );
    item->setMarked( marked );

    if( marked )
    {
      markParentDirectories( item );
      fileCount++;
      emit comparedFileData( item );
    }

    if( step++ % DISPLAY_UPDATE_PERIOD == (DISPLAY_UPDATE_PERIOD-1) )
      qApp->processEvents();
    
    item = resultList.next();
  }

  emit statusInfo( i18n( "File number:%1" ).arg( fileCount ) );
}

bool Synchronizer::totalSizes( int * leftCopyNr, KIO::filesize_t *leftCopySize, int * rightCopyNr,
                               KIO::filesize_t *rightCopySize, int *deleteNr, KIO::filesize_t *deletableSize )
{
  bool hasAnythingToDo = false;
  
  *leftCopySize = *rightCopySize = *deletableSize = 0;
  *leftCopyNr = *rightCopyNr = *deleteNr = 0;
  
  SynchronizerFileItem *item = resultList.first();

  while( item )
  {
    if( item->isMarked() )
    {
      switch( item->task() )
      {
      case TT_COPY_TO_LEFT:
        *leftCopySize += item->rightSize();
        (*leftCopyNr)++;
        hasAnythingToDo = true;
        break;
      case TT_COPY_TO_RIGHT:
        *rightCopySize += item->leftSize();
        (*rightCopyNr)++;
        hasAnythingToDo = true;
        break;
      case TT_DELETE:
        *deletableSize += item->leftSize();
        (*deleteNr)++;
        hasAnythingToDo = true;
        break;
      default:
        break;
      }
    }
    item = resultList.next();
  }

  return hasAnythingToDo;
}

void Synchronizer::synchronize( bool leftCopyEnabled, bool rightCopyEnabled, bool deleteEnabled,
                                bool overWrite )
{
    this->leftCopyEnabled   = leftCopyEnabled;
    this->rightCopyEnabled  = rightCopyEnabled;
    this->deleteEnabled     = deleteEnabled;
    this->overWrite         = overWrite;

    autoSkip = paused = false;

    leftCopyNr = rightCopyNr = deleteNr = 0;
    leftCopySize = rightCopySize = deleteSize = 0;

    currentTask = resultList.first();
    executeTask();
}

void Synchronizer::executeTask()
{
  TaskType task;
  
  do {
    if( currentTask == 0 )
    {
      emit synchronizationFinished();
      return;
    }
    
    if( currentTask->isMarked() )
    {
      task = currentTask->task();
      
      if( leftCopyEnabled && task == TT_COPY_TO_LEFT )
        break;
      else if( rightCopyEnabled && task == TT_COPY_TO_RIGHT )
        break;
      else if( deleteEnabled && task == TT_DELETE )
        break;
    }

    currentTask = resultList.next();
  }while( true );

  QString dirName = currentTask->directory();
  if( !dirName.isEmpty() )
    dirName += "/";
  
  leftURL  = fromPathOrURL( leftBaseDir + dirName + currentTask->name() );
  rightURL = fromPathOrURL( rightBaseDir + dirName + currentTask->name() );
  
  switch( task )
  {
  case TT_COPY_TO_LEFT:
    if( currentTask->isDir() )
    {
      KIO::SimpleJob *job = KIO::mkdir( leftURL );
      connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(slotTaskFinished(KIO::Job*)));
    }
    else
    {
      KURL destURL( leftURL );
      if( !currentTask->destination().isNull() )
        destURL = fromPathOrURL( currentTask->destination() );
      
      KIO::FileCopyJob *job = KIO::file_copy(rightURL, destURL, -1,
                                overWrite || currentTask->overWrite(), false, false );
      connect(job,SIGNAL(processedSize (KIO::Job *, KIO::filesize_t )), this,
                  SLOT  (slotProcessedSize (KIO::Job *, KIO::filesize_t )));
      connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(slotTaskFinished(KIO::Job*)));
    }
    break;
  case TT_COPY_TO_RIGHT:
    if( currentTask->isDir() )
    {
      KIO::SimpleJob *job = KIO::mkdir( rightURL );
      connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(slotTaskFinished(KIO::Job*)));
    }
    else
    {
      KURL destURL( rightURL );
      if( !currentTask->destination().isNull() )
        destURL = fromPathOrURL( currentTask->destination() );

      KIO::FileCopyJob *job = KIO::file_copy(leftURL, destURL, -1,
                                overWrite || currentTask->overWrite(), false, false );
      connect(job,SIGNAL(processedSize (KIO::Job *, KIO::filesize_t )), this,
                  SLOT  (slotProcessedSize (KIO::Job *, KIO::filesize_t )));
      connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(slotTaskFinished(KIO::Job*)));
    }
    break;
  case TT_DELETE:
    {
      KIO::DeleteJob *job = KIO::del( leftURL, false );
      connect(job,SIGNAL(result(KIO::Job*)),this,SLOT(slotTaskFinished(KIO::Job*)));
    }
    break;
  default:
    break;
  }
}

void Synchronizer::slotTaskFinished(KIO::Job *job )
{
  do {
    if( job->error() )
    {
      if( job->error() == KIO::ERR_FILE_ALREADY_EXIST && currentTask->task() != TT_DELETE )
      {
        KIO::RenameDlg_Result result;
        QString newDest;

        if( autoSkip )
          break;  
      
        if ( currentTask->task() == TT_COPY_TO_LEFT )
        {
          result = Observer::self()->open_RenameDlg ( job, i18n("File Already Exists"), rightURL.path(),
            leftURL.path(), (KIO::RenameDlg_Mode)( KIO::M_OVERWRITE | KIO::M_SKIP | KIO::M_MULTI ), newDest,
            currentTask->rightSize(), currentTask->leftSize(), (time_t)-1, (time_t)-1,
            currentTask->rightDate(), currentTask->leftDate());
        }
        else
        {
          result = Observer::self()->open_RenameDlg ( job, i18n("File Already Exists"), leftURL.path(),
            rightURL.path(), (KIO::RenameDlg_Mode)( KIO::M_OVERWRITE | KIO::M_SKIP | KIO::M_MULTI ), newDest,
            currentTask->leftSize(), currentTask->rightSize(), (time_t)-1, (time_t)-1,
            currentTask->leftDate(), currentTask->rightDate());          
        }

        switch ( result )
        {
        case KIO::R_RENAME:
          currentTask->setDestination( newDest );
          executeTask();
          return;
        case KIO::R_OVERWRITE:
          currentTask->setOverWrite();
          executeTask();
          return;
        case KIO::R_OVERWRITE_ALL:
          overWrite = true;
          executeTask();
          return;
        case KIO::R_AUTO_SKIP:
          autoSkip = true;
        case KIO::R_SKIP:
        default:
          break;
        }
        break;
      }

      if( job->error() != KIO::ERR_DOES_NOT_EXIST || currentTask->task() != TT_DELETE )
      {
        if( autoSkip )
          break;
          
        QString error;

        switch( currentTask->task() )
        {
        case TT_COPY_TO_LEFT:
          error = i18n("Error at copying file %1 to %2!").arg( rightURL.path() ).arg( leftURL.path() );
          break;
        case TT_COPY_TO_RIGHT:
          error = i18n("Error at copying file %1 to %2!").arg( leftURL.path() ).arg( rightURL.path() );
          break;
        case TT_DELETE:
          error = i18n("Error at deleting file %1!").arg( leftURL.url() );
          break;
        default:
          break;
        }

        KIO::SkipDlg_Result result = Observer::self()->open_SkipDlg( job, true, error );

        switch( result )
        {
        case KIO::S_CANCEL:
          executeTask();  /* simply retry */
          return;
        case KIO::S_AUTO_SKIP:
          autoSkip = true;
        default:
          break;
        }
      }
    }
  }while( false );

  switch( currentTask->task() )
  {
  case TT_COPY_TO_LEFT:
    leftCopyNr++;
    leftCopySize += currentTask->rightSize();
    break;
  case TT_COPY_TO_RIGHT:
    rightCopyNr++;
    rightCopySize += currentTask->leftSize();
    break;
  case TT_DELETE:
    deleteNr++;
    deleteSize += currentTask->leftSize();
    break;
  default:
    break;
  }

  emit processedSizes( leftCopyNr, leftCopySize, rightCopyNr, rightCopySize, deleteNr, deleteSize );
    
  currentTask = resultList.next();

  if( paused )
    emit pauseAccepted();
  else
    executeTask();
}

void Synchronizer::slotProcessedSize( KIO::Job * , KIO::filesize_t size)
{
  KIO::filesize_t dl = 0, dr = 0, dd = 0;

  switch( currentTask->task() )
  {
  case TT_COPY_TO_LEFT:
    dl = size;
    break;
  case TT_COPY_TO_RIGHT:
    dr = size;
    break;
  case TT_DELETE:
    dd = size;
    break;
  default:
    break;
  }
  
  emit processedSizes( leftCopyNr, leftCopySize+dl, rightCopyNr, rightCopySize+dr, deleteNr, deleteSize+dd );
}

void Synchronizer::pause()
{
  paused = true;
}

void Synchronizer::resume()
{
  paused = false;
  executeTask();
}

KURL Synchronizer::fromPathOrURL( QString origin )
{
  QString password, loginName;
  
  // breakdown the url;
  /* FIXME: untill KDE fixes the bug we have to check for
     passwords and users with @ in them... */
  bool bugfix = origin.find("@") != origin.findRev("@");
  if(bugfix){
    if(origin.find(":") != origin.findRev(":")){
      int passStart = origin.find( ":",origin.find(":")+1 )+1;
      int passLen = origin.findRev("@")-passStart;
      password = origin.mid(passStart,passLen);
      origin = origin.remove(passStart-1,passLen+1);
    }
    if(origin.find("@") != origin.findRev("@")){
      int usrStart = origin.find( "/" )+1;
      if(origin.at(usrStart) == '/') ++usrStart;
      int usrLen = origin.findRev("@")-usrStart;
      loginName = origin.mid(usrStart,usrLen);
      origin = origin.remove(usrStart,usrLen+1);
    }
  }
  KURL url = origin;
  if(loginName.isEmpty()) loginName = url.user();
  if(password.isEmpty())  password  = url.pass();
  if(bugfix){
   url.setPass(password);
    url.setUser(loginName);
  }

  return url;
}
