/*
cmultiprogresswidget.ui/h/cpp
-----------------------------
Begin: 2019-06-18
Author: Aaron Reeves <aaron@aaronreeves.com>
---------------------------------------------------
Copyright (C) 2019 - 2021 Aaron Reeves

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, version 3.
*/

#include "cmultiprogresswidget.h"
#include "ui_cmultiprogresswidget.h"

#include <QClipboard>

#include <ar_general_purpose/arcommon.h>

CMultiProgressWidget::CMultiProgressWidget( QWidget *parent ) : QWidget(parent), _mutex( QMutex::Recursive ), ui(new Ui::CMultiProgressWidget) {
  ui->setupUi(this);

  cancelClicked = false;
  ui->btnCancel->setEnabled( false );
  ui->btnOK->setVisible( false );
  ui->btnCopy->setEnabled( false );
  connect( ui->btnCancel, SIGNAL( clicked(bool) ), this, SLOT( setProcessingCanceled() ) );
  connect( ui->btnOK, SIGNAL( clicked(bool) ), this, SIGNAL( okButtonClicked() ) );
  connect( ui->btnCopy, SIGNAL( clicked(bool) ), this, SLOT( copyMessages() ) );

}


CMultiProgressWidget::CMultiProgressWidget( const QStringList& stepNames, QWidget *parent ) : QWidget(parent), _mutex( QMutex::Recursive ), ui(new Ui::CMultiProgressWidget) {
  ui->setupUi(this);

  cancelClicked = false;
  ui->btnOK->setVisible( false );
  ui->btnCopy->setEnabled( false );
  connect( ui->btnCancel, SIGNAL( clicked(bool) ), this, SLOT( setProcessingCanceled() ) );
  connect( ui->btnOK, SIGNAL( clicked(bool) ), this, SIGNAL( okButtonClicked() ) );
  connect( ui->btnCopy, SIGNAL( clicked(bool) ), this, SLOT( copyMessages() ) );

  setup( stepNames );
}


CMultiProgressWidget::~CMultiProgressWidget() {
  delete ui;
}


void CMultiProgressWidget::setProcessingCanceled() {
  ui->lblCaption->setText( QStringLiteral( "Terminating process..." ) );
  cancelClicked = true;
  ui->btnCancel->setEnabled( false );
  emit cancelButtonClicked();
}


void CMultiProgressWidget::setup( const QStringList& stepNames ) {
  while( !_progressWidgetsList.isEmpty() ) {
    delete _progressWidgetsList.takeFirst();
  }

  if( nullptr != ui->frame->layout() ) {
    delete ui->frame->layout();
  }

  QVBoxLayout* layout = new QVBoxLayout();

  for( int i = 0; i < stepNames.count(); ++i ) {
    CProgressWidget* wid = new CProgressWidget();
    _progressWidgetsList.append( wid );
    _progressWidgetsHash.insert( stepNames.at(i), wid );
    layout->addWidget( wid );
  }
  ui->frame->setLayout( layout );

  this->start();
}


void CMultiProgressWidget::setup( const QStringList& stepNames, const QStringList& captions ) {
  Q_ASSERT( stepNames.count() == captions.count() );

  setup( stepNames );

  for( int i = 0; i < captions.count(); ++i ) {
    setCaption( i, captions.at(i) );
  }
}


void CMultiProgressWidget::setCaption( QString caption ) {
  _mutex.lock();

  if( !  cancelClicked ) {
    _mainCaption = caption;
    ui->lblCaption->setText( caption );
  }

  _mutex.unlock();
}


void CMultiProgressWidget::start() {
  _mutex.lock();

  for( int i = 0; i < _progressWidgetsList.count(); ++i ) {
    _progressWidgetsList.at(i)->restart();
  }

  cancelClicked = false;

  if( !_mainCaption.isEmpty() ) {
    setCaption( _mainCaption );
  }

  this->clearMessages();

  ui->btnOK->setVisible( false );
  ui->btnCancel->setVisible( true );
  ui->btnCancel->setEnabled( true );

  _mutex.unlock();
}


void CMultiProgressWidget::start( int idx, int nSteps, QString caption ) {
  _mutex.lock();
  Q_ASSERT( ( 0 <= idx ) && ( idx < _progressWidgetsList.count() ) );
  _progressWidgetsList.at(idx)->start( nSteps, caption );
  _mutex.unlock();
}
void CMultiProgressWidget::start( QString key, int nSteps, QString caption ) {
  _mutex.lock();
  Q_ASSERT( _progressWidgetsHash.contains( key ) );
  _progressWidgetsHash.value(key)->start( nSteps, caption );
  _mutex.unlock();
}


void CMultiProgressWidget::setCaption(int idx, QString caption ) {
  _mutex.lock();
  Q_ASSERT( ( 0 <= idx ) && ( idx < _progressWidgetsList.count() ) );
  _progressWidgetsList.at(idx)->setCaption( caption );
  _mutex.unlock();
}
void CMultiProgressWidget::setCaption( QString stepName, QString caption ) {
  _mutex.lock();
  Q_ASSERT( _progressWidgetsHash.contains( stepName ) );
  _progressWidgetsHash.value(stepName)->setCaption( caption );
  _mutex.unlock();
}


void CMultiProgressWidget::setNSteps( int idx, int nSteps ) {
  _mutex.lock();
  Q_ASSERT( ( 0 <= idx ) && ( idx < _progressWidgetsList.count() ) );
  _progressWidgetsList.at(idx)->setMaximum( nSteps );
  _mutex.unlock();
}
void CMultiProgressWidget::setNSteps( QString stepName, int nSteps ) {
  _mutex.lock();
  Q_ASSERT( _progressWidgetsHash.contains( stepName ) );
  _progressWidgetsHash.value(stepName)->setMaximum( nSteps );
  _mutex.unlock();
}


void CMultiProgressWidget::setStepComplete( int idx, int step ) {
  _mutex.lock();
  Q_ASSERT( ( 0 <= idx ) && ( idx < _progressWidgetsList.count() ) );
  _progressWidgetsList.at(idx)->setValue( step );
  _mutex.unlock();
}
void CMultiProgressWidget::setStepComplete( QString stepName, int step ) {
  _mutex.lock();
  Q_ASSERT( _progressWidgetsHash.contains( stepName ) );
  _progressWidgetsHash.value(stepName)->setValue( step );
  _mutex.unlock();
}


void CMultiProgressWidget::incrementSteps( int idx ) {
  _mutex.lock();
  Q_ASSERT( ( 0 <= idx ) && ( idx < _progressWidgetsList.count() ) );
  _progressWidgetsList.at(idx)->setValue( _progressWidgetsList.at(idx)->value() + 1 );
  _mutex.unlock();
}
void CMultiProgressWidget::incrementSteps( QString stepName ) {
  _mutex.lock();
  Q_ASSERT( _progressWidgetsHash.contains( stepName ) );
  _progressWidgetsHash.value(stepName)->setValue( _progressWidgetsHash.value(stepName)->value() + 1 );
  _mutex.unlock();
}

void CMultiProgressWidget::setProgressComplete( int idx, int result ) {
  _mutex.lock();
  Q_ASSERT( ( 0 <= idx ) && ( idx < _progressWidgetsList.count() ) );
  _progressWidgetsList.at(idx)->setComplete( result );
  _mutex.unlock();
}
void CMultiProgressWidget::setProgressComplete( QString stepName, int result ) {
  _mutex.lock();
  Q_ASSERT( _progressWidgetsHash.contains( stepName ) );
  _progressWidgetsHash.value(stepName)->setComplete( result );
  _mutex.unlock();
}


void CMultiProgressWidget::setAllProgressComplete( int result ) {
  _mutex.lock();

  if( cancelClicked ) {
    ui->lblCaption->setText( QStringLiteral( "Processing terminated." ) );
  }
  else if( result & ReturnCode::PROCESSING_INTERRUPTED ) {
    ui->lblCaption->setText( QStringLiteral( "Processing terminated." ) );
  }
  else if( ReturnCode::SUCCESS == result ) {
    ui->lblCaption->setText( QStringLiteral( "Processing successful." ) );
  }
  else {
    ui->lblCaption->setText( QStringLiteral( "Processing completed with errors." ) );
  }

  for( int i = 0; i < _progressWidgetsList.count(); ++i ) {
    _progressWidgetsList.at(i)->setComplete( result );
  }

  ui->btnCancel->setVisible( false );
  ui->btnOK->setVisible( true );

  _mutex.unlock();
}


void CMultiProgressWidget::clearMessages() {
  _mutex.lock();
  ui->pteMessages->clear();
  ui->btnCopy->setEnabled( false );
  _mutex.unlock();
}


void CMultiProgressWidget::appendMessage( const QString msg ) {
  _mutex.lock();
  ui->pteMessages->appendPlainText( msg );
  ui->btnCopy->setEnabled( true );
  _mutex.unlock();
}


void CMultiProgressWidget::copyMessages() {
  QGuiApplication::clipboard()->setText( ui->pteMessages->toPlainText() );
}


//void CMultiProgressWidget::setRunner( CRunner* runner ) {
//  _runner = runner;
//  connectSlots();
//}


//void CMultiProgressWidget::connectSlots() {
////  if( nullptr != _runner ) {
////    connect( _runner, SIGNAL( processingStarted() ), this, SLOT( processingStarted() ) );
////    connect( _runner, SIGNAL( setNFiles( int )    ), this, SLOT( setNFiles( int )    ) );
////    connect( _runner, SIGNAL( setNSheets( int )   ), this, SLOT( setNSheets( int )   ) );

////    connect( _runner, SIGNAL( fileProcessingStarted() ), this, SLOT( fileProcessingStarted() ) );

////    connect( _runner, SIGNAL( operationStart( QString, int ) ), this, SLOT( operationStart( QString, int ) ) );
////    connect( _runner, SIGNAL( operationProgress( int )       ), this, SLOT( operationProgress( int )       ) );
////    connect( _runner, SIGNAL( operationComplete()            ), this, SLOT( operationComplete()            ) );
////    connect( _runner, SIGNAL( lastOperationComplete()        ), this, SLOT( lastOperationComplete()        ) );

////    connect( _runner, SIGNAL( fileProcessingComplete() ), this, SLOT( fileProcessingComplete() ) );

////    connect( _runner, SIGNAL( processingComplete()   ), this, SLOT( processingComplete()   ) );
////    connect( _runner, SIGNAL( processingTerminated() ), this, SLOT( processingTerminated() ) );
////  }

//  connect( &appLog, SIGNAL( messageLogged( QString ) ), this, SLOT( appendMessage( QString ) ) );
//}


//void CMultiProgressWidget::disconnectSlots() {
////  if( nullptr != _runner ) {
////    disconnect( _runner, SIGNAL( processingStarted() ), this, SLOT( processingStarted() ) );
////    disconnect( _runner, SIGNAL( setNFiles( int )    ), this, SLOT( setNFiles( int )    ) );
////    disconnect( _runner, SIGNAL( setNSheets( int )   ), this, SLOT( setNSheets( int )   ) );

////    disconnect( _runner, SIGNAL( fileProcessingStarted() ), this, SLOT( fileProcessingStarted() ) );

////    disconnect( _runner, SIGNAL( operationStart( QString, int ) ), this, SLOT( operationStart( QString, int ) ) );
////    disconnect( _runner, SIGNAL( operationProgress( int )       ), this, SLOT( operationProgress( int )       ) );
////    disconnect( _runner, SIGNAL( operationComplete()            ), this, SLOT( operationComplete()            ) );
////    disconnect( _runner, SIGNAL( lastOperationComplete()        ), this, SLOT( lastOperationComplete()        ) );

////    disconnect( _runner, SIGNAL( fileProcessingComplete() ), this, SLOT( fileProcessingComplete() ) );

////    disconnect( _runner, SIGNAL( processingComplete()   ), this, SLOT( processingComplete()    ) );
////    disconnect( _runner, SIGNAL( processingTerminated() ), this, SLOT( processingTerminated()  ) );
////  }

//  disconnect( &appLog, SIGNAL( messageLogged( QString ) ), this, SLOT( appendMessage( QString ) ) );
//}

