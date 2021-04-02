#include "QtAdditions/QtUtilities.h"


#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/qerrormessage.h>
#include <QtWidgets/qmessagebox.h>
#include <QtWidgets/qinputdialog.h>

#include <fstream>

namespace QtAdditions
{
   using namespace std;

   // Ask yes/no/cancel.
   YesNoCancel AskYesNoCancel(const QString& title, const QString& text, QWidget* parent)
   {
      QMessageBox box;
      box.setWindowTitle(title);
      box.setText(text);
      box.setStandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::Cancel | QMessageBox::StandardButton::No );
      box.setDefaultButton(QMessageBox::StandardButton::Cancel);
      switch (box.exec())
      {
         case QMessageBox::StandardButton::Yes:
            return YesNoCancel::Yes;
         case QMessageBox::StandardButton::No:
            return YesNoCancel::No;
         case QMessageBox::StandardButton::Cancel:
         default:
            return YesNoCancel::Cancel;
      }
   }

   wstring AskForText(const QString& title, const QString& label, const QString& initialContent, QWidget* parent)
   {
      QString text = QInputDialog::getText(parent, title, label, QLineEdit::Normal, initialContent);

      return text.toStdWString();
   }
}

// vim: sw=3 : sts=3 : et : sta :
