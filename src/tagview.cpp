#include "tagview.h"
#include "pyrosqt.h"
#include "pyrosdb.h"
#include "tagitem.h"
#include "globbing.h"
#include "tagtreemodel.h"
#include "configtab.h"

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
        options.widget->style()->drawControl(QStyle::CE_ItemViewItem, &options, painter);


        if (model->data(index,TagTreeModel::TYPE_ROLE).toInt() == TagTreeModel::ALIAS_TAG){
            painter->setPen(QColor(130,180,250));
            options.rect.adjust(options.fontMetrics.boundingRect(options.text).width(),0,0,0);
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
    sortByColumn(0,Qt::AscendingOrder);

    contextMenu = new QMenu(this);
    setContextMenuPolicy(Qt::CustomContextMenu);

    contextMenu->addAction("Copy Tag",                this, &TagView::copy_tag);
    contextMenu->addAction("Search in new tab",       this, &TagView::create_search_with_selected_tags);
    contextMenu->addAction("Add Parent",              this, &TagView::add_parent);
    contextMenu->addAction("Add Alias",               this, &TagView::add_alias);
    contextMenu->addAction("Add Child",               this, &TagView::add_child);
    contextMenu->addAction("Remove Tag",              this, &TagView::remove_tag);
    contextMenu->addAction("Remove Tag Relationship", this, &TagView::remove_relationship);

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


void TagView::remove_relationship(){
    QVector<QByteArray> tags;

    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();

    QAbstractItemModel *model = this->model();

    foreach(QModelIndex index, indexes) {
        QModelIndex parent =  index.parent();
        if (parent.isValid()){
            tags.push_back(model->data(index,TagTreeModel::TAG_ROLE).toByteArray());
            tags.push_back(model->data(parent,TagTreeModel::TAG_ROLE).toByteArray());
        }
    }

    if (tags.count() > 1){
        PyrosTC *ptc = PyrosTC::get();
        ptc->remove_relationship(tags);
    }
}

void TagView::add_alias(){
    PyrosTC *ptc = PyrosTC::get();
    QVector<QByteArray> selected_tags = get_selected_tags();
    QVector<QByteArray> ext_tags = create_tag_dialog("Alias");
    ptc->add_alias(selected_tags,ext_tags);
}

void TagView::add_parent(){
    PyrosTC *ptc = PyrosTC::get();
    QVector<QByteArray> selected_tags = get_selected_tags();
    QVector<QByteArray> ext_tags = create_tag_dialog("Parent");
    ptc->add_parent(selected_tags,ext_tags);

}

void TagView::add_child(){
    PyrosTC *ptc = PyrosTC::get();
    QVector<QByteArray> selected_tags = get_selected_tags();
    QVector<QByteArray> ext_tags = create_tag_dialog("Child");
    ptc->add_child(selected_tags,ext_tags);
}

QVector<QByteArray> TagView::get_selected_tags(){
    QVector<QByteArray> selected_tags;

    QItemSelectionModel *select = selectionModel();
    QModelIndexList indexes = select->selectedIndexes();

    foreach(QModelIndex index, indexes) {
        QVariant varient = model()->data(index,TagTreeModel::TAG_ROLE);
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

    if (file_hash.isEmpty()){
        QAbstractItemModel *model = this->model();
        std::sort(indexes.begin(), indexes.end(), std::less<QModelIndex>());
        for (int i = indexes.length()-1; i >= 0;i--){
            QModelIndex index = indexes.at(i);
            if (index.parent() == rootIndex())
                model->removeRow(index.row(),index.parent());
        }
    }
}

void TagView::remove_tag_from_view(QVector<QByteArray> hashes,QVector<QByteArray> tags)
{
    foreach(QByteArray hash, hashes)
        if (hash == file_hash)
            foreach(QByteArray tag, tags)
                for(int i = 0; i < tag_model->rowCount();i++)
                    if (tag_model->data(tag_model->index(i,0),TagTreeModel::TAG_ROLE) == tag)
                        tag_model->removeRow(i);
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
        QString str = model->data(index,TagTreeModel::TAG_ROLE).toString();
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

bool TagView::generateModel(PyrosTag **pt,size_t cur,size_t max,const QModelIndex index,
                            int type)
{
    QAbstractItemModel *model = tag_model;

    for (size_t i = 0; i < max; i++) {
        if (cur == pt[i]->par){
            if (!model->insertRows(0,1,index))
                return false;

            const QModelIndex child = model->index(0,0,index);


            QString str = QString::fromUtf8(pt[i]->tag);

            if (!model->setData(child,str,TagTreeModel::TAG_ROLE))
                return false;

            bool result;
            if (pt[i]->isAlias)
                result = model->setData(child,TagTreeModel::ALIAS_TAG,TagTreeModel::TYPE_ROLE);
            else
                result = model->setData(child,type,TagTreeModel::TYPE_ROLE);

            if (!result || !generateModel(pt,i,max,child,type))
                return false;
        }
    }
    return true;

}

bool TagView::add_unloaded_tag_to_model(QString tag)
{
    QAbstractItemModel *model = tag_model;
    QModelIndex index = QModelIndex();

    if (!model->insertRows(0,1,index))
        return false;

    const QModelIndex child = model->index(0,0,index);

    if (!model->setData(child,tag,TagTreeModel::TAG_ROLE) || !model->setData(child,TagTreeModel::TEMP_TAG,TagTreeModel::TYPE_ROLE))
        return false;

    return true;

}


void TagView::clear()
{
    QAbstractItemModel *model = this->model();
    model->removeRows(0,model->rowCount());
}

void TagView::add_tags_by_hash(QVector<QByteArray> hashes,QVector<QByteArray> tags)
{
    foreach(QByteArray hash,hashes){
        if (hash == file_hash){
            add_tags(tags);
            break;
        }
    }
}

void TagView::add_tags(QVector<QByteArray> tags)
{
    foreach(QByteArray tag,tags)
        add_unloaded_tag_to_model(tag);

    if (file_hash.isEmpty()){
        PyrosTC *ptc = PyrosTC::get();
        PyrosTC::related_cb  cb= [&](QVector<PyrosList*> related_tags,QVector<QByteArray> unfound_tags){
            replace_temp_tags(related_tags,unfound_tags);
            foreach(PyrosList *tags, related_tags)
                Pyros_List_Free(tags,(Pyros_Free_Callback)Pyros_Free_Tag);
        };
        ptc->get_related_tags(this,tags,cb,tag_type);
    }

}


void TagView::replace_temp_tags(QVector<PyrosList*> related_tags,QVector<QByteArray> unfound_tags)
{

    foreach(QByteArray tag,unfound_tags){
        for(int i = 0; i < tag_model->rowCount();i++){
            TagTreeModel::TAG_TYPE type = TagTreeModel::NEW_TAG;
            const QModelIndex index = tag_model->index(i,0);
            if (tag_model->data(index,TagTreeModel::TAG_ROLE) == tag){

                if (tag_type == PYROS_SEARCH_RELATIONSHIP){
                    type = TagTreeModel::INVALID_TAG;
                    if(tag.contains('*') || tag.contains('?') ||
                            (tag.contains('[') && tag.contains(']'))){
                        type = TagTreeModel::SPECIAL_TAG;
                    } else {
                        for	(unsigned j = 0; j < sizeof(PYROS_SEARCH_KEYWORDS)/sizeof(*PYROS_SEARCH_KEYWORDS);j++){
                            if (tag.startsWith(QByteArray(PYROS_SEARCH_KEYWORDS[j])+':')){
                                type = TagTreeModel::SPECIAL_TAG;
                                break;
                            }
                        }
                    }

                }

                tag_model->update_tag_type(index,type);
            }
        }
    }

    foreach(PyrosList *tags,related_tags){
        for(int i = 0; i < tag_model->rowCount();i++){
            const QModelIndex index = tag_model->index(i,0);

            if (tag_model->data(index,TagTreeModel::TYPE_ROLE) == TagTreeModel::TEMP_TAG &&
                 tag_model->data(index,TagTreeModel::TAG_ROLE).toByteArray() == ((PyrosTag*)tags->list[0])->tag){
                tag_model->update_tag_type(index,TagTreeModel::NORMAL_TAG);
                generateModel((PyrosTag**)tags->list,0,tags->length,index,TagTreeModel::NORMAL_TAG);
                break;
            }
        }
    }
}


void TagView::setTagsFromFile(PyrosFile *file)
{
    if (file != nullptr){
        PyrosTC *ptc = PyrosTC::get();

        PyrosTC::tag_cb cb = [&](QVector<PyrosTag*> tags){
            clear();
            setSortingEnabled(false);
            generateModel(tags.data(),-1,tags.length(),QModelIndex(),TagTreeModel::NORMAL_TAG);
            setSortingEnabled(true);

            foreach(PyrosTag *tag, tags)
                Pyros_Free_Tag(tag);
        };

        ptc->get_tags_from_hash(this,file->hash, cb);
        file_hash = file->hash;
    }
}

void TagView::add_related_tags_to_view_recursively(QVector<QByteArray> tags,QVector<QByteArray> related_tags,uint type,QModelIndex parent)
{
    for(int i = 0; i < tag_model->rowCount(parent);i++){
        QModelIndex index = tag_model->index(i,0,parent);
        bool tag_matched = false;

        foreach(QByteArray tag,tags){
            if (tag_model->data(index,TagTreeModel::TAG_ROLE) == tag){
                foreach(QByteArray related_tag,related_tags){
                    if (tag_model->insertRows(0,1,index)){
                        const QModelIndex child = tag_model->index(0,0,index);
                        tag_model->setData(child,related_tag,TagTreeModel::TAG_ROLE);
                        tag_model->setData(child, (type & PYROS_ALIAS) ?TagTreeModel::ALIAS_TAG : TagTreeModel::NORMAL_TAG,TagTreeModel::TYPE_ROLE);
                    }
                }
                tag_matched = true;
                break;
            }
        }

        if (!tag_matched)
            add_related_tags_to_view_recursively(tags,related_tags,type,index);

    }

}

void TagView::add_related_tags_to_view(QVector<QByteArray> tags,QVector<QByteArray> related_tags,uint type)
{
    if (tag_type & type)
        add_related_tags_to_view_recursively(tags,related_tags,type,QModelIndex());
}

void TagView::remove_related_tags_from_view_recursively(QVector<QByteArray> tag_pairs, QModelIndex parent,bool tag_found)
{
    for(int i = 0; i < tag_model->rowCount(parent);i++){
    reset:
        QModelIndex index = tag_model->index(i,0,parent);
        bool tag_matched = false;

        for(int j = 1; j < tag_pairs.count();j+=2){
            QString tag = tag_model->data(index,TagTreeModel::TAG_ROLE).toString();

            if ((tag == tag_pairs.at(j) || tag == tag_pairs.at(j-1))){
                tag_matched = true;
                if (tag_found){
                    QString parent_tag = tag_model->data(parent,TagTreeModel::TAG_ROLE).toString();

                    if ((parent_tag == tag_pairs.at(j) || parent_tag == tag_pairs.at(j-1))){
                        tag_model->removeRow(index.row(),parent);
                        goto reset;
                    }
                }
            }
        }

        remove_related_tags_from_view_recursively(tag_pairs,index,tag_matched);
    }
}

void TagView::remove_related_tags_from_view(QVector<QByteArray> tag_pairs)
{
    remove_related_tags_from_view_recursively(tag_pairs,QModelIndex(),false);
}

void TagView::setTagType(int type)
{
    tag_type = type;
    connect(PyrosTC::get(), &PyrosTC::tag_relationship_added, this, &TagView::add_related_tags_to_view);
    connect(PyrosTC::get(), &PyrosTC::tag_relationship_removed, this, &TagView::remove_related_tags_from_view);
    if (type == PYROS_FILE_RELATIONSHIP){
        connect(PyrosTC::get(), &PyrosTC::tag_added, this, &TagView::add_tags_by_hash);
        connect(PyrosTC::get(), &PyrosTC::tags_added_with_related, this, &TagView::replace_temp_tags);
        connect(PyrosTC::get(), &PyrosTC::tag_removed, this, &TagView::remove_tag_from_view);
    }


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
            selected_tags[i] = Globbing::escape_glob_characters(selected_tags[i]);

    emit new_search_with_selected_tags(selected_tags);
}

void TagView::add_selected_tags_to_search()
{
    QVector<QByteArray> selected_tags = get_selected_tags();

    for (int i = 0; i < selected_tags.count(); i++)
        selected_tags[i] = Globbing::escape_glob_characters(selected_tags[i]);

    emit add_tag_to_current_search(selected_tags);

}
void TagView::filter_selected_tags_from_search()
{
    QVector<QByteArray> selected_tags = get_selected_tags();

    for (int i = 0; i < selected_tags.count(); i++)
        selected_tags[i] = '-'+Globbing::escape_glob_characters(selected_tags[i]);

    emit add_tag_to_current_search(selected_tags);
}

void TagView::highlight_similar_tags_recursively(const QString &text,const QModelIndex parent)
{
    if (text.isEmpty()){
        for(int i = 0; i < tag_model->rowCount(parent);i++){
            QModelIndex index = tag_model->index(i,0,parent);
            tag_model->set_tag_highlight(index,false);
            highlight_similar_tags_recursively(text,index);
        }
        return;
    }

    for(int i = 0; i < tag_model->rowCount(parent);i++){
        QModelIndex index = tag_model->index(i,0,parent);
        QString tag = tag_model->data(index,TagTreeModel::TAG_ROLE).toString();
        tag_model->set_tag_highlight(index,tag.startsWith(text));
        highlight_similar_tags_recursively(text,index);
    }

}

void TagView::highlight_similar_tags(const QString &text)
{
    bool use_highlights = configtab::setting_value(configtab::HIGHLIGHT_SIMMILAR_TAGS).value<bool>();

    if (use_highlights)
        highlight_similar_tags_recursively(text,QModelIndex());
}
