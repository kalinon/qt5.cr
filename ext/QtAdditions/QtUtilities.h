#pragma once

#include <functional>
#include <filesystem>
#include <string>

#include <QtGui/qicon.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qbitmap.h>

#include <QtWidgets/qaction.h>
#include <QtWidgets/qtoolbutton.h>

namespace QtAdditions
{

   ////////////////////////////////////////////////////////////////////////////
   //
   // Ask yes/no/cancel.

   enum class YesNoCancel
   {
      No = 0,
      Yes = 1,
      Cancel = 2,
   };

   YesNoCancel AskYesNoCancel(const QString& title, const QString& text, QWidget* parent);

   ////////////////////////////////////////////////////////////////////////////
   //
   // Disable feedback in a widget.

   struct DisableFeedback
   {
      DisableFeedback(QWidget* widget, int& recursionCounter)
      : _widget(widget), _recursionCounter(recursionCounter)
      {
         if (_widget)
         {
            _recursionCounter++;
            _widget->blockSignals(_recursionCounter > 0);
         }
      }

      ~DisableFeedback()
      {
         if (_widget)
         {
            _recursionCounter--;
            _widget->blockSignals(_recursionCounter > 0);
         }
      }

   private:
      QWidget* _widget;
      int& _recursionCounter;
   };
}

// vim: sw=3 : sts=3 : et : sta :
