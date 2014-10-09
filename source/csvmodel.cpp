#include "csvmodel.h"
#include <QList>
#include <QVector>
#include <QStringList>
#include <QTextStream>
#include <QDebug>


class VariantTable {
    int m_row_count;
    int m_column_count;
    QVector< QVector<QVariant> > m_content;
public:
    VariantTable(int row_count = 0, int column_count = 0) :
        m_row_count(row_count),
        m_column_count(column_count)
    {
        m_content.fill(QVector<QVariant>(m_column_count, QVariant()), m_row_count);
    }

    ~VariantTable() {}

    inline int rowCount() const { return m_row_count; }
    inline int columnCount() const { return m_column_count; }

    void setRowCount(int row_count)
    {
        if (m_row_count != row_count) {
            int previous_row_count = m_row_count;
            m_row_count = row_count;
            m_content.resize(m_row_count);
            for (int i = previous_row_count; i < m_row_count; ++ i)
                m_content[i] = QVector<QVariant>(m_column_count, QVariant());
        }
    }

    void setColumnCount(int column_count)
    {
        if (m_column_count != column_count) {
            m_column_count = column_count;
            for (int i = 0; i < m_row_count; ++ i)
                m_content[i].resize(m_column_count);
        }
    }

    inline QVariant itemAt(int row, int column) const { return m_content[row][column]; }
    inline void setItem(int row, int column, const QVariant &value) { m_content[row][column] = value; }

    void addRow() { setRowCount(m_row_count + 1); }
    void addColumn() { setColumnCount(m_column_count + 1); }

    void clear() { m_content.clear(); }
};


class CSVModelPrivate {
public:
    QChar separator;
    bool has_header;
    bool strip_quotes;
    QVector<QVariant> header_data;
    VariantTable model_data;
public:
    CSVModelPrivate() {}
    ~CSVModelPrivate() {}

    void clear()
    {
        separator = QChar();
        has_header = false;
        header_data.clear();
        model_data.clear();
    }

    QStringList splitBySeparator(const QString &source_str) const
    {
        QStringList res;

        QChar single_quote = QChar('\'');
        QChar double_quote = QChar('\"');

        QString single_quoted_end_marker;
        single_quoted_end_marker.append(single_quote);
        single_quoted_end_marker.append(separator);

        QString double_quoted_end_marker;
        double_quoted_end_marker.append(double_quote);
        double_quoted_end_marker.append(separator);

        QString csv_str = source_str;
        QString section;

        while (!csv_str.isEmpty()) {
            int section_end_index = 0;

            if (csv_str.startsWith(single_quote)) {
                section_end_index = csv_str.indexOf(single_quoted_end_marker);
                section_end_index = (section_end_index != -1) ? section_end_index + 1
                                                              : section_end_index;
            }
            else if (csv_str.startsWith(double_quote)) {
                section_end_index = csv_str.indexOf(double_quoted_end_marker);
                section_end_index = (section_end_index != -1) ? section_end_index + 1
                                                              : section_end_index;
            }
            else {
                section_end_index = csv_str.indexOf(separator);
            }

            section = csv_str.left(section_end_index);

            res.append(section);

            csv_str = csv_str.mid(section_end_index + 1);

            if (section_end_index == -1)
                break;
        }

        return res;
    }

    QString stripQuotes(const QString &source_str) const
    {
        QChar single_quote = QChar('\'');
        QChar double_quote = QChar('\"');

        QString stripped_str = source_str;

        if ((stripped_str.startsWith(single_quote) && stripped_str.endsWith(single_quote)) ||
            (stripped_str.startsWith(double_quote) && stripped_str.endsWith(double_quote)))
        {
            stripped_str.remove(0, 1);
            stripped_str.chop(1);
        }

        return stripped_str;
    }
};


CSVModel::CSVModel(QObject *parent) :
    QAbstractTableModel(parent),
    d_ptr(new CSVModelPrivate())
{
    Q_D(CSVModel);
    d->separator = QChar();
    d->has_header = false;
    d->strip_quotes = true;
}

CSVModel::~CSVModel()
{
    delete d_ptr;
}

QChar CSVModel::separator() const
{
    Q_D(const CSVModel);
    return d->separator;
}

bool CSVModel::hasHeader() const
{
    Q_D(const CSVModel);
    return d->has_header;
}

bool CSVModel::stripQuotes() const
{
    Q_D(const CSVModel);
    return d->strip_quotes;
}

void CSVModel::setStripQuotes(bool on)
{
    Q_D(CSVModel);
    if (d->strip_quotes != on) {
        d->strip_quotes = on;
        emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
        if (d->has_header)
            emit headerDataChanged(Qt::Horizontal, 0, columnCount() - 1);
    }
}

void CSVModel::parse(QTextStream *source_stream, const QChar &separator, bool has_header)
{
    Q_D(CSVModel);

    beginResetModel();

    d->clear();

    if (source_stream == 0) {
        endResetModel();
        return;
    }

    if (source_stream->atEnd()) {
        endResetModel();
        return;
    }

    d->separator = separator;
    d->has_header = has_header;

    int row_number = 0;

    if (d->has_header) {
        QStringList header = d->splitBySeparator(source_stream->readLine());

        d->header_data.resize(header.size());

        for (int i = 0; i < header.size(); ++ i)
            d->header_data[i] = header.at(i);

        d->model_data.setColumnCount(header.size());
    }
    else {
        QStringList first_row = d->splitBySeparator(source_stream->readLine());

        d->model_data.setColumnCount(first_row.size());
        d->model_data.addRow();

        for (int i = 0; i < first_row.size(); ++ i)
            d->model_data.setItem(row_number, i, first_row.at(i));

        ++ row_number;
    }

    while (!source_stream->atEnd()) {
        QStringList row = d->splitBySeparator(source_stream->readLine());

        if (row.size() != d->model_data.columnCount()) {
            qDebug() << row_number << "size mismatch. Needed" << d->model_data.columnCount()
                     << ", got" << row.size();
        }

        d->model_data.addRow();

        for (int i = 0; (i < d->model_data.columnCount()) && (i < row.size()); ++ i)
            d->model_data.setItem(row_number, i, row.at(i));

        ++ row_number;
    }

    endResetModel();
}

void CSVModel::clear()
{
    Q_D(CSVModel);
    beginResetModel();
    d->clear();
    endResetModel();
}

int CSVModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    Q_D(const CSVModel);
    return d->model_data.rowCount();
}

int CSVModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    Q_D(const CSVModel);
    return d->model_data.columnCount();
}

QVariant CSVModel::data(const QModelIndex &index, int role) const
{
    Q_D(const CSVModel);

    if (!index.isValid())
        return QVariant();

    if (index.row() >= d->model_data.rowCount())
        return QVariant();

    if (index.column() >= d->model_data.columnCount())
        return QVariant();

    if (role == Qt::DisplayRole) {
        return d->strip_quotes ? QVariant(d->stripQuotes(d->model_data.itemAt(index.row(), index.column()).toString()))
                               : d->model_data.itemAt(index.row(), index.column());
    }

    return QVariant();
}

QVariant CSVModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_D(const CSVModel);

    if (!d->has_header)
        return QVariant();

    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Vertical)
            return QString::number(section + 1);

        if (orientation == Qt::Horizontal) {
            if (d->has_header && (section < d->header_data.size())) {
                return d->strip_quotes ? QVariant(d->stripQuotes(d->header_data.at(section).toString()))
                                       : d->header_data.at(section);
            }
        }
    }

    return QVariant();
}
