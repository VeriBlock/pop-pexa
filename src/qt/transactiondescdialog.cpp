// Copyright (c) 2011-2016 The Bitcoin Core developers
// Copyright (c) 2019 The Pexa Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <qt/transactiondescdialog.h>
#include <ui_transactiondescdialog.h>

#include <qt/transactiontablemodel.h>

#include <qt/guiutil.h>
#include <QModelIndex>

TransactionDescDialog::TransactionDescDialog(const QModelIndex &idx, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransactionDescDialog)
{
    ui->setupUi(this);
    setWindowTitle(tr("Details for %1").arg(idx.data(TransactionTableModel::TxIDRole).toString()));
    QString desc = idx.data(TransactionTableModel::LongDescriptionRole).toString();
    ui->detailText->setHtml(desc);

    ui->detailText->setFont(GUIUtil::getSubLabelFont());
}

TransactionDescDialog::~TransactionDescDialog()
{
    delete ui;
}
