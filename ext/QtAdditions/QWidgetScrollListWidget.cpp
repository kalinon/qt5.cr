#include "QtAdditions/QWidgetScrollListWidget.h"
#include "QtAdditions/QWidgetListWidget.h"
#include "QtAdditions/QWidgetListMimeData.h"

namespace QtAdditions
{
   QWidgetScrollListWidget::QWidgetScrollListWidget(QWidget* widget, QWidget* parent)
   : QScrollArea(parent)
   {
      setWidget(widget);

      bool isVertical = true;
      if (auto list = dynamic_cast<QWidgetListWidget*>(widget))
         if (!list->isVertical())
            isVertical = false;

      if (isVertical)
      {
         setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
         setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
         setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
      }
      else
      {
         setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
         setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
         setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum));
      }

      setWidgetResizable(true);
      setSizeAdjustPolicy(SizeAdjustPolicy::AdjustToContents);
   }

}

