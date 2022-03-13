/*
cfileselect.h/cpp
-----------------
Begin: 2008/02/18
Author: Aaron Reeves <aaron.reeves@naadsm.org>
--------------------------------------------------
Copyright (C) 2008 - 2019 Aaron Reeves

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
*/


#ifndef CFileSelect_H
#define CFileSelect_H

#include <QtWidgets>
#include <QtCore>

namespace Ui {
  class CFileSelect;
}

class CMyLineEdit : public QLineEdit {
  friend class CFileSelect;
  Q_OBJECT
  public:
    CMyLineEdit( QWidget* parent = nullptr );

  protected slots:
    void setActualPathName( const QString& val );

  protected:
    void focusInEvent( QFocusEvent* event );
    void focusOutEvent( QFocusEvent* event );
    void mousePressEvent( QMouseEvent* event );
    void mouseReleaseEvent( QMouseEvent* event);

    bool _firstClick;
    QString _actualPathName;
    QString _initialDirectory;
};


class CFileSelect : public QWidget {
  Q_OBJECT

  public:
    enum FileDialogModes {
      ModeUnspecified  = 0x0000,
      ModeOpenFile     = 0x0001,
      ModeSaveFile     = 0x0002,
      ModeAnyFile      = 0x0004,
      ModeExistingFile = 0x0008,
      ModeExistingDir  = 0x0010
    };

    explicit CFileSelect( QWidget* parent = nullptr, Qt::WindowFlags f = nullptr );
    ~CFileSelect();

    // Properties
    //-----------
    // Mode MUST be specified.  Everything else has a reasonable default.
    void setMode( const int mode ) { _mode = mode; }
    int mode() const { return _mode; }

    // Filter format: "Plain text files (*.txt);; All Files (*.*)"
    // Defaults to all files.
    void setFilter( const QString& val ) { _filter = val; }
    QString specifiedFilter() const { return _filter; }
    QString selectedFilter() const { return _selectedFilter; }

    // Label displayed by the widget itself
    void setLabel( const QString& val );
    QString label() const;

    // Message displayed by file dialog boxes
    void setCaption( const QString& val ) { _caption = val; }
    QString caption() const { return _caption; }

    // FIXME: Make these play nicely with isEnabled.
    void setReadOnly(const bool readOnly );
    bool isReadOnly() const { return _isReadOnly; }

    // The functions that do the work
    //-------------------------------
    QString filePath() const;  // Returns the file name, including the path
    QString directory() const;
    QString fileName() const { if( fileExists() ) return QFileInfo( filePath() ).fileName(); else return QString(); }
    //QString filename() const { return filePath(); } // FIXME: Make this go away!  Use filePath() instead, for consistency with QFileInfo.
    QString fileType() const { if( fileExists() ) return QFileInfo( filePath() ).suffix(); else return QString(); }

    void setFilePath( const QString& val );
    void setDirectory( const QString& val );
    //void setFilename( const QString& val ) // FIXME: Make this go away!  Use setFilePath() instead, for consistency with QFileInfo.

    void clearPath(); // Blows away any specified path
    bool isEmpty() const; // Indicates whether a path (either file or directory) has been set.

    bool dirExists() const;
    bool fileExists() const;

    // FIXME: Make this a real function!
    bool fileIsValid() { return true; }

  public slots:
    void debug();

  protected slots:
    void selectFileOrFolder(); // Called when the button is clicked
    //void updateLineEdit( QString newPathName );
    void slotEditingFinished();

  signals:
    void pathNameChanged( QString newPathName );

  protected:
    void selectFolder();
    void selectFile();
    QString startDirectory() const; // Helper used by selectFolder and selectFile

    QString _filter;
    QString _selectedFilter;
    QString _caption;
    int _mode;

    QString _lastUsedPath;

    bool _isReadOnly;

  private:
    Ui::CFileSelect *ui;
};

#endif // CFileSelect_H
