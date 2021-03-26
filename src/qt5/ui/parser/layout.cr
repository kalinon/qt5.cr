module Qt::Ui
  class Parser
    module Layout
      abstract def logger

      getter layouts : Set(Qt::Layout) = Set(Qt::Layout).new

      # Return generated `Qt::Layout` by name, or nil if not found
      def get_layout(name : String) : Qt::Layout?
        self.layouts.find &.object_name == name
      end

      # Convert Qt class string to a crystal `Qt::Layout` instance. Will set *parent* for the created node.
      private def layout_from_class(klass : String, parent : Qt::Widget) : Qt::Layout?
        case klass
        when "QGridLayout"
          Qt::GridLayout.new(parent)
        when "QFormLayout"
          Qt::FormLayout.new(parent)
        when "QBoxLayout"
          Qt::BoxLayout.new(Qt::BoxLayout::Direction::LeftToRight, parent)
        when "QHBoxLayout"
          Qt::HBoxLayout.new(parent)
        when "QVBoxLayout"
          Qt::VBoxLayout.new(parent)
        when "QStackedLayout"
          Qt::StackedLayout.new(parent)
        else
          logger.warn { "widget #{klass} is not supported" }
          nil
        end
      end

      # Parse the XML layout node
      private def parse_layout_node(node : XML::Node, parent : Qt::Widget) : Qt::Layout?
        layout = layout_from_class(node["class"], parent)
        if layout.nil?
          logger.warn &.emit("unable to create layout", klass: node["class"], name: node["name"])
          return
        else
          # Append new layout to our list
          self.layouts << layout
          logger.info &.emit("created layout",
            crystal_klass: layout.class.to_s,
            klass: node["class"],
            name: node["name"],
            parent: parent.object_name
          )
        end
        layout.object_name = node["name"] if node["name"]?

        case layout
        when Qt::GridLayout
          parse_grid_layout_node(node, parent, layout)
        else
          logger.warn { "Unsupported layout: #{layout.class}" }
        end

        layout
      end

      # Parse the XML node for the provided `Qt::GridLayout` *layout*
      private def parse_grid_layout_node(node : XML::Node, parent : Qt::Widget, layout : Qt::GridLayout)
        node.children.select(&.element?).each do |child|
          case child.name
          when "item"
            row, column = child["row"].to_i, child["column"].to_i
            child.children.select(&.element?).each do |n|
              item = parse_xml_node(n, parent)
              case item
              when Qt::Widget
                logger.debug { "Adding widget #{item.object_name} to #{layout.object_name} at #{row} #{column}" }
                layout.add_widget(item, row, column, 1, 1)
              when Qt::LayoutItem
                logger.debug { "Adding item #{item} to #{layout.object_name} at #{row} #{column}" }
                layout.add_item(item, row, column, 1, 1)
              end
            end
          else
            logger.warn { "grid layout sub node #{child.name} is not supported" }
          end
        end
      end
    end
  end
end
