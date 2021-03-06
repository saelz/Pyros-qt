#include "tagview.h"
#include "pyrosqt.h"
#include "pyrosdb.h"
#include "tagitem.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QInputDialog>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include <QSettings>
#include <QPainter>


#include <QDebug>

class TagDelegate : public QStyledItemDelegate{
public:
    QAbstractItemModel *model;
    TagDelegate(QAbstractItemModel *model,QObject *parent)
        : QStyledItemDelegate(parent),model(model){}

    void paint(QPainter* painter,const QStyleOptionViewItem &option,const QModelIndex &index) const{
        QStyleOptionViewItem options = option;
        QColor color = model->data(index,Qt::ForegroundRole).value<QColor>();

        initStyleOption(&options, index);

        painter->save();


        painter->setPen(color);
        //painter->drawText(options.rect,tag);
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);


        if (model->data(index,Qt::EditRole).toInt() == TagItem::ALIAS_TAG){
            painter->setPen(QColor(130,180,250));
            options.rect.adjust(options.fontMetrics.boundingRect(options.text).width(),0,0,0);
            //options.text = " <A>";
            //options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);
            painter->drawText(options.rect,"  <A>");
        }

        painter->restore();
    }

};

TagView::TagView(QWidget *parent) :
    QTreeView(parent)
{

    tag_model = new TagTreeModel("Tag",this);
    sort_model = new QSortFilterProxyModel(this);

    sort_model->setSourceModel(tag_model);
    setModel(sort_model);

    setSelectionMode(QAbstractItemView::ExtendedSelection);
    sortByColumn(TagItem::TAG_COLUMN,Qt::AscendingOrder);

    contextMenu = new QMenu(this);
    setContextMenuPolicy(Qt::CustomContextMenu);

    contextMenu->addAction("Copy Tag",          this, &TagView::copy_tag);
    contextMenu->addAction("Search in new tab", this, &TagView::create_search_with_selected_tags);
    contextMenu->addAction("Add Parent",        this, &TagView::add_parent);
    contextMenu->addAction("Add Alias",         this, &TagView::add_alias);
    contextMenu->addAction("Add Child",         this, &TagView::add_child);
    contextMenu->addAction("Remove Tag",        this, &TagView::remove_tag);
    contextMenu->addAction("Remove Ext",        this, &TagView::remove_ext);

    QAction *copy_bind = new QAction("Copy Tag",this);
    copy_bind->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    copy_bind->setShortcut(QKeySequence("CTRL+C"));
    addAction(copy_bind);

    connect(copy_bind,&QAction::triggered,this,&TagView::copy_tag);

    connect(this, &TagView::customContextMenuRequested, this, &TagView::onCustomContextMenu);

    setItemDelegate(new TagDelegate(sort_model,this));
}


TagView::~TagView()
{
    delete contextMenu;
    delete sort_model;
    delete tag_model;
}


void TagView::add_tag_as_child(TagItem::TAG_TYPE type,QString tag){
    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();
    QAbstractItemModel *model = this->model();

    foreach(QModelIndex index, indexes) {
        int insert_location = model->rowCount(index)-1;
        if (insert_location < 0)
            insert_location = 0;

        if (model->insertRows(insert_location,1,index)){

            const QModelIndex child = model->index(model->rowCount(index)-1,TagItem::TAG_COLUMN,index);
            const QModelIndex tag_type = model->index(model->rowCount(index)-1,TagItem::TYPE_COLUMN,index);
            model->setData(child,tag);
            model->setData(tag_type,type);
        }
    }
}

void TagView::remove_ext(){
    QVector<QByteArray> tags;

    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();

    QAbstractItemModel *model = this->model();
    std::sort(indexes.begin(), indexes.end(), std::less<QModelIndex>());

    foreach(QModelIndex index, indexes) {
        QModelIndex parent =  index.parent();
        if (parent.isValid()){
            QByteArray ext_parent = model->data(parent,Qt::DisplayRole).toByteArray();
            QByteArray ext = model->data(index,Qt::DisplayRole).toByteArray();
            model->removeRow(index.row(),index.parent());
            tags.push_back(ext);
            tags.push_back(ext_parent);
        }
    }
    PyrosTC *ptc = PyrosTC::get();
    ptc->remove_ext(tags);
}

void TagView::add_alias(){
    PyrosTC *ptc = PyrosTC::get();
    QVector<QByteArray> selected_tags = get_selected_tags();
    QVector<QByteArray> ext_tags = create_tag_dialog("Alias");
    ptc->add_alias(selected_tags,ext_tags);
    foreach (QByteArray tag,ext_tags)
        add_tag_as_child(TagItem::ALIAS_TAG,tag);
}
void TagView::add_parent(){
    PyrosTC *ptc = PyrosTC::get();
    QVector<QByteArray> selected_tags = get_selected_tags();
    QVector<QByteArray> ext_tags = create_tag_dialog("Parent");
    ptc->add_parent(selected_tags,ext_tags);
    if (tag_type == PYROS_FILE_RELATIONSHIP){
        foreach (QByteArray tag,ext_tags)
            add_tag_as_child(TagItem::NORMAL_TAG,tag);
    }
}
void TagView::add_child(){
    PyrosTC *ptc = PyrosTC::get();
    QVector<QByteArray> selected_tags = get_selected_tags();
    QVector<QByteArray> ext_tags = create_tag_dialog("Child");
    ptc->add_child(selected_tags,ext_tags);
    if (tag_type == PYROS_SEARCH_RELATIONSHIP){
        foreach (QByteArray tag,ext_tags)
            add_tag_as_child(TagItem::NORMAL_TAG,tag);
    }
}

QVector<QByteArray> TagView::get_selected_tags(){
    QVector<QByteArray> selected_tags;

    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();

    foreach(QModelIndex index, indexes) {
        QVariant varient = model()->data(index,Qt::DisplayRole);
        selected_tags.append(varient.toByteArray());
    }
    return selected_tags;
}

QVector<QByteArray> TagView::create_tag_dialog(QByteArray title){
    QVector<QByteArray> tags;
    bool ok;
    QString text_return = QInputDialog::getText(this, tr("QInputDialog::getText()"),
                                         title, QLineEdit::Normal,
                                         "", &ok);
    if (!ok || text_return.isEmpty())
        return tags;

    QByteArray text = text_return.toUtf8();
    QList<QByteArray> l = text.split('\n');
    tags = l.toVector();
    return tags;
}

void TagView::remove_tag()
{
    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();

    QVector<QByteArray> tags = get_selected_tags();
    emit removeTag(tags);

    QAbstractItemModel *model = this->model();
    std::sort(indexes.begin(), indexes.end(), std::less<QModelIndex>());
    for (int i = indexes.length()-1; i >= 0;i--){
        QModelIndex index = indexes.at(i);
        if (index.parent() == rootIndex())
            model->removeRow(index.row(),index.parent());
    }


}

void TagView::copy_tag()
{
    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();
    QModelIndex index;

    QAbstractItemModel *model = this->model();
    QClipboard *clipboard = QGuiApplication::clipboard();

    QString tags = "";
    bool multiple = false;

    foreach(index, indexes) {
        QString str = model->data(index,Qt::DisplayRole).toString();
        if (multiple){
            tags += "\n" + str;
        } else {
            tags = str;

            multiple = true;
        }
    }

    clipboard->setText(tags);
}

void TagView::onCustomContextMenu(const QPoint &point)
{
    QModelIndex index = indexAt(point);
    if (index.isValid()) {
        contextMenu->exec(viewport()->mapToGlobal(point));
    }
}

bool TagView::loadModelFromTag(QByteArray tag,const QModelIndex index,PyrosDB *pyrosDB)
{
    if (!tag.isEmpty()){

        PyrosList *tags;
        bool result;

        if (!tag.compare("*"))
            tags = nullptr;
        else
            tags = Pyros_Get_Related_Tags(pyrosDB,tag.data(),tag_type);


        if (tags == nullptr || tags->length == 0){
            PyrosTag *pt = new PyrosTag;
            TagItem::TAG_TYPE type;

            if (tag_type == PYROS_FILE_RELATIONSHIP){
                type = TagItem::NEW_TAG;
            }else{
                if(tag.contains('*') || tag.contains('?') ||
                        (tag.contains('[') && tag.contains(']')) ||
                        tag.startsWith("mime:") ||
                        tag.startsWith("ext:") ||
                        tag.startsWith("order:") ||
                        tag.startsWith("tagcount:") ||
                        tag.startsWith("limit:") ||
                        tag.startsWith("page:") ||
                        tag.startsWith("size:")
                        )
                    type = TagItem::SPECIAL_TAG;
                else
                    type = TagItem::INVALID_TAG;
            }

            pt->par = -1;
            pt->tag = tag.data();
            pt->isAlias = false;
            result = generateModel(&pt,-1,1,index,type);
            delete pt;
        } else {
            result = generateModel((PyrosTag**)tags->list,-1,tags->length,index);

            Pyros_List_Free(tags,(Pyros_Free_Callback)Pyros_Free_Tag);
        }

        return result;
    }

    return true;
}

bool TagView::generateModel(PyrosTag **pt,size_t cur,size_t max,const QModelIndex index,
                            TagItem::TAG_TYPE type)
{
    QAbstractItemModel *model = tag_model;

    for (size_t i = 0; i < max; i++) {
        if (cur == pt[i]->par){
            if (!model->insertRows(0,1,index))
                return false;

            const QModelIndex child = model->index(0,TagItem::TAG_COLUMN,index);
            const QModelIndex tag_type = model->index(0,TagItem::TYPE_COLUMN,index);


            QString str = QString::fromUtf8(pt[i]->tag);

            if (!model->setData(child,str))
                return false;

            bool result;
            if (pt[i]->isAlias){
                result = model->setData(tag_type,TagItem::ALIAS_TAG);
            } else {
                result = model->setData(tag_type,type);
            }

            if (!result || !generateModel(pt,i,max,child))
                return false;
        }
    }
    return true;

}

void TagView::clear()
{
    QAbstractItemModel *model = this->model();
    model->removeRows(0,model->rowCount());
}

void TagView::add_tags(QVector<QByteArray> tags)
{
    PyrosTC *ptc = PyrosTC::get();

    PyrosDB *pyrosDB = Pyros_Open_Database(ptc->db_path());
    foreach(QByteArray tag,tags){
        loadModelFromTag(tag,QModelIndex(),pyrosDB);
    }

    Pyros_Close_Database(pyrosDB);

}

void TagView::setTagsFromFile(PyrosFile *file)
{
    if (file != nullptr){
        PyrosTC *ptc = PyrosTC::get();

        PyrosTC::tag_cb cb = [&](QVector<PyrosTag*> tags){
            clear();
            setSortingEnabled(false);
            generateModel(tags.data(),-1,tags.length(),QModelIndex());
            setSortingEnabled(true);

            foreach(PyrosTag *tag, tags)
                Pyros_Free_Tag(tag);
        };

        ptc->get_tags_from_hash(this,file->hash, cb);
    }
}

void TagView::setTagType(int type)
{
    tag_type = type;
}

void TagView::append_search_options_to_contenxt_menu()
{
    contextMenu->addAction("Add tag to search",      this, &TagView::add_selected_tags_to_search);
    contextMenu->addAction("Filter tag from search", this, &TagView::filter_selected_tags_from_search);
}

void TagView::create_search_with_selected_tags()
{
    QVector<QByteArray> selected_tags = get_selected_tags();

    if (tag_type == PYROS_FILE_RELATIONSHIP)
        for (int i = 0; i < selected_tags.count(); i++)
            selected_tags[i] = PyrosTC::escape_glob_characters(selected_tags[i]);

    emit new_search_with_selected_tags(selected_tags);
}

void TagView::add_selected_tags_to_search()
{
    QVector<QByteArray> selected_tags = get_selected_tags();

    for (int i = 0; i < selected_tags.count(); i++)
        selected_tags[i] = PyrosTC::escape_glob_characters(selected_tags[i]);

    emit add_tag_to_current_search(selected_tags);

}
void TagView::filter_selected_tags_from_search()
{
    QVector<QByteArray> selected_tags = get_selected_tags();

    for (int i = 0; i < selected_tags.count(); i++)
        selected_tags[i] = '-'+PyrosTC::escape_glob_characters(selected_tags[i]);

    emit add_tag_to_current_search(selected_tags);
}
