#include "task.h"
//----------------------------------------------------------------------------------------------
#include "utils/settings.h"
//----------------------------------------------------------------------------------------------
/*!
 * \brief Идентификатор задачи, увеличивается на 1 с каждым таском
 */
static quint64 g_task_id = 0;
//----------------------------------------------------------------------------------------------

EteraTask::EteraTask() : QObject(), QRunnable()
{
    m_api   = NULL;
    m_token = EteraSettings::instance()->token();
    m_id    = EteraTask::nextID();

    m_success_mutex = NULL;
}
//----------------------------------------------------------------------------------------------

EteraTask::~EteraTask()
{
    // последний шанс, если не было удаления в cleanup
    delete m_api;
}
//----------------------------------------------------------------------------------------------

quint64 EteraTask::nextID()
{
    g_task_id++;
    return g_task_id;
}
//----------------------------------------------------------------------------------------------