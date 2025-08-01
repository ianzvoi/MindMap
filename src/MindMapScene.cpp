#include "MindMapScene.h"
#include "Connection.h"
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QAction>
#include <QInputDialog>
#include <QMessageBox>
#include <QGraphicsItem>
#include <QStyle>
#include <queue>

MindMapScene::MindMapScene(QObject* parent)
    : QGraphicsScene(parent), m_mode(None), m_tempNode(nullptr)
{
}

void MindMapScene::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        if (m_mode == AddNode) {
            // 添加新节点
            bool ok;
            QString text = QInputDialog::getText(nullptr, "添加节点", "输入节点内容:",
                                                 QLineEdit::Normal, "新节点", &ok);
            if (ok && !text.isEmpty()) {
                MindMapNode* node = addNode(text);
                node->setPos(event->scenePos());
                clearSelection();
                node->setSelected(true);
            }
            event->accept();
            return;
        }
        else if (m_mode == AddConnection) {
            // 开始连接节点
            QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
            if (item && item->type() == MindMapNode::Type) {
                m_tempNode = static_cast<MindMapNode*>(item);
                event->accept();
                return;
            }
        }
    }
    else if (event->button() == Qt::RightButton) {
        // 显示上下文菜单
        QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
        if (item && item->type() == MindMapNode::Type) {
            showNodeContextMenu(static_cast<MindMapNode*>(item), event->screenPos());
            event->accept();
            return;
        }
        else {
            showSceneContextMenu(event->screenPos(), event->scenePos());
            event->accept();
            return;
        }
    }

    QGraphicsScene::mousePressEvent(event);
}

void MindMapScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_mode == AddConnection && m_tempNode) {
        // 完成连接节点
        QGraphicsItem* item = itemAt(event->scenePos(), QTransform());
        if (item && item->type() == MindMapNode::Type) {
            MindMapNode* endNode = static_cast<MindMapNode*>(item);
            if (m_tempNode != endNode) {
                // 创建连接
                Connection* connection = new Connection(m_tempNode, endNode);
                addItem(connection);
                connection->updatePath();
            }
        }
        m_tempNode = nullptr;
        event->accept();
        return;
    }

    QGraphicsScene::mouseReleaseEvent(event);
}

void MindMapScene::showNodeContextMenu(MindMapNode* node, const QPoint& screenPos)
{
    QMenu menu;

    QAction* editAction = menu.addAction("编辑节点");
    QAction* deleteAction = menu.addAction("删除节点");
    QAction* colorAction = menu.addAction("设置颜色");
    menu.addSeparator();
    QAction* addChildAction = menu.addAction("添加子节点");

    QAction* selectedAction = menu.exec(screenPos);

    if (selectedAction == editAction) {
        bool ok;
        QString text = QInputDialog::getText(nullptr, "编辑节点", "输入新内容:",
                                             QLineEdit::Normal, node->getText(), &ok);
        if (ok && !text.isEmpty()) {
            node->setText(text);
        }
    }
    else if (selectedAction == deleteAction) {
        // 删除节点及其所有连接
        removeNode(node);
    }
    else if (selectedAction == colorAction) {
        // 在实际应用中应使用颜色选择对话框
        node->setColor(QColor(rand() % 256, rand() % 256, rand() % 256));
    } else if (selectedAction == addChildAction) {
        bool ok;
        QString text = QInputDialog::getText(nullptr, "添加子节点", "输入节点内容:",
                                             QLineEdit::Normal, "子节点", &ok);
        if (ok && !text.isEmpty()) {

        }
    }
}

void MindMapScene::showSceneContextMenu(const QPoint& screenPos, const QPointF& scenePos)
{
    QMenu menu;

    QAction* addNodeAction = menu.addAction("添加节点");
    QAction* saveAction = menu.addAction("保存思维导图");
    QAction* loadAction = menu.addAction("加载思维导图");

    QAction* selectedAction = menu.exec(screenPos);

    if (selectedAction == addNodeAction) {
        bool ok;
        QString text = QInputDialog::getText(nullptr, "添加节点", "输入节点内容:",
                                             QLineEdit::Normal, "新节点", &ok);
        if (ok && !text.isEmpty()) {
            MindMapNode* node = addNode(text);
            node->setPos(screenPos);
        }
    }
    else if (selectedAction == saveAction) {
        QMessageBox::information(nullptr, "保存功能", "保存功能将在后续版本中实现");
    }
    else if (selectedAction == loadAction) {
        QMessageBox::information(nullptr, "加载功能", "加载功能将在后续版本中实现");
    }
}

MindMapNode* MindMapScene::addNode(const QString &name)
{
    MindMapNode* node = new MindMapNode(name);
    if(node != nullptr){
        addItem(node);
    }
    return node;
}

// delete node and edges connect to it.
void MindMapScene::removeNode(MindMapNode* node)
{
    if (!node) return;

    // 删除连接
    QList<Connection*> connections = node->connections();
    for (Connection* connection : connections) {
        removeItem(connection);
        delete connection;
    }
    
    // 删除节点本身
    removeItem(node);
    delete node;
}



#include<iostream>
void MindMapScene::genSceneFromFolder(std::string folder_path){
    std::filesystem::directory_entry folder(folder_path);
    using di = std::filesystem::directory_iterator;
    namespace fs = std::filesystem;
    using queue = std::queue<std::pair<fs::directory_entry,MindMapNode*>>;
    queue bfs_queue;
    
    if(folder.exists() && !folder.is_directory()){
        return;
    }
    try {
        auto parentNode = addNode(QString(folder.path().c_str()));
        parentNode->setColor(QColor(2,20,102));

        bfs_queue.push({folder,parentNode});
        while(!bfs_queue.empty()){
            const auto parent = bfs_queue.front();
            for(auto sub : di(parent.first)){
                auto childNode = addNode(QString(sub.path().stem().c_str()));

                double theta = (rand() % 360) * M_PI / 180.0;
                childNode->setPos(parent.second->pos()+300*QPointF(cos(theta),sin(theta)));
                
                if(sub.is_directory()){
                    bfs_queue.push({sub,childNode});
                    childNode->setColor(QColor(0,102,204));
                } else {
                    // dosth.
                }
                auto edge = new Connection(parent.second,childNode);
                addItem(edge);
                edge->updatePath();
            }
            bfs_queue.pop(); 

        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "目录迭代失败: " << e.what() 
                  << "\n - 路径: " << e.path1() 
                  << "\n - 错误码: " << e.code().message() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "标准异常: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "未知异常类型" << std::endl;
    } 
    return;
}
