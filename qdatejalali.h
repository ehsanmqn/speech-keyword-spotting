#ifndef QDATEJALALI_H
#define QDATEJALALI_H

#include <QStringList>

class QDateJalali
{
public:
    QDateJalali();
    int div(int a,int b);
    QStringList ToShamsi(QString year, QString month,QString day );
};

#endif // QDATEJALALI_H
