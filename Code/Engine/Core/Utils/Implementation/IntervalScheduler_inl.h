
#include <Foundation/SimdMath/SimdRandom.h>

EZ_ALWAYS_INLINE ezUInt32 ezIntervalSchedulerBase::GetHistogramIndex(ezTime value)
{
  constexpr ezUInt32 maxSlotIndex = HistogramSize - 1;
  const double x = ezMath::Max((value - m_MinInterval).GetSeconds() * m_fInvIntervalRange, 0.0);
  const double i = ezMath::Sqrt(x) * maxSlotIndex;
  return ezMath::Min(static_cast<ezUInt32>(i), maxSlotIndex);
}

EZ_ALWAYS_INLINE ezTime ezIntervalSchedulerBase::GetHistogramSlotValue(ezUInt32 uiIndex)
{
  constexpr double norm = 1.0 / (HistogramSize - 1.0);
  const double x = uiIndex * norm;
  return (x * x) * (m_MaxInterval - m_MinInterval) + m_MinInterval;
}

// static
EZ_ALWAYS_INLINE float ezIntervalSchedulerBase::GetRandomZeroToOne(int pos, ezUInt32& seed)
{
  return ezSimdRandom::FloatZeroToOne(ezSimdVec4i(pos), ezSimdVec4u(seed++)).x();
}

constexpr ezTime s_JitterRange = ezTime::MakeFromMicroseconds(10);

// static
EZ_ALWAYS_INLINE ezTime ezIntervalSchedulerBase::GetRandomTimeJitter(int pos, ezUInt32& seed)
{
  const float x = ezSimdRandom::FloatZeroToOne(ezSimdVec4i(pos), ezSimdVec4u(seed++)).x();
  return s_JitterRange * (x * 2.0f - 1.0f);
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
bool ezIntervalScheduler<T>::Data::IsValid() const
{
  return m_Interval.IsZeroOrPositive();
}

template <typename T>
void ezIntervalScheduler<T>::Data::MarkAsInvalid()
{
  m_Interval = ezTime::MakeFromSeconds(-1);
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
void ezIntervalScheduler<T>::AddOrUpdateWork(const T& work, ezTime interval)
{
  typename DataMap::Iterator it;
  if (m_WorkIdToData.TryGetValue(work, it))
  {
    auto& data = it.Value();
    ezTime oldInterval = data.m_Interval;
    if (interval == oldInterval)
      return;

    data.MarkAsInvalid();

    const ezUInt32 uiHistogramIndex = GetHistogramIndex(oldInterval);
    m_Histogram[uiHistogramIndex]--;
  }

  Data data;
  data.m_Work = work;
  data.m_Interval = ezMath::Max(interval, ezTime::MakeZero());
  data.m_DueTime = m_CurrentTime + GetRandomZeroToOne(m_Data.GetCount(), m_uiSeed) * data.m_Interval;
  data.m_LastScheduledTime = m_CurrentTime;

  m_WorkIdToData[work] = InsertData(data);

  const ezUInt32 uiHistogramIndex = GetHistogramIndex(data.m_Interval);
  m_Histogram[uiHistogramIndex]++;
}

template <typename T>
void ezIntervalScheduler<T>::RemoveWork(const T& work)
{
  typename DataMap::Iterator it;
  if (m_WorkIdToData.Remove(work, &it))
  {
    auto& data = it.Value();
    ezTime oldInterval = data.m_Interval;
    data.MarkAsInvalid();

    const ezUInt32 uiHistogramIndex = GetHistogramIndex(oldInterval);
    m_Histogram[uiHistogramIndex]--;
  }
}

template <typename T>
ezTime ezIntervalScheduler<T>::GetInterval(const T& work) const
{
  typename DataMap::Iterator it;
  EZ_VERIFY(m_WorkIdToData.TryGetValue(work, it), "Entry not found");
  return it.Value().m_Interval;
}

template <typename T>
void ezIntervalScheduler<T>::Update(ezTime deltaTime, RunWorkCallback runWorkCallback)
{
  if (deltaTime <= ezTime::MakeZero())
    return;

  m_CurrentTime += deltaTime;

  if (m_Data.IsEmpty())
  {
    m_fNumWorkToSchedule = 0.0;
  }
  else
  {
    double fNumWork = 0;
    for (ezUInt32 i = 0; i < HistogramSize; ++i)
    {
      fNumWork += (1.0 / ezMath::Max(m_HistogramSlotValues[i], deltaTime).GetSeconds()) * m_Histogram[i];
    }
    fNumWork *= deltaTime.GetSeconds();

    if (m_fNumWorkToSchedule == 0.0)
    {
      m_fNumWorkToSchedule = fNumWork;
    }
    else
    {
      // running average of num work per update to prevent huge spikes
      m_fNumWorkToSchedule = ezMath::Lerp<double>(m_fNumWorkToSchedule, fNumWork, 0.05);
    }

    const float fRemainder = static_cast<float>(ezMath::Fraction(m_fNumWorkToSchedule));
    const int pos = static_cast<int>(m_CurrentTime.GetNanoseconds());
    const ezUInt32 extra = GetRandomZeroToOne(pos, m_uiSeed) < fRemainder ? 1 : 0;
    const ezUInt32 uiScheduleCount = ezMath::Min(static_cast<ezUInt32>(m_fNumWorkToSchedule) + extra, m_Data.GetCount());

    // schedule work
    {
      auto it = m_Data.GetIterator();
      for (ezUInt32 i = 0; i < uiScheduleCount; ++i, ++it)
      {
        auto& data = it.Value();
        if (data.IsValid())
        {
          if (runWorkCallback.IsValid())
          {
            runWorkCallback(data.m_Work, m_CurrentTime - data.m_LastScheduledTime);
          }

          // add a little bit of random jitter so we don't end up with perfect timings that might collide with other work
          data.m_DueTime = m_CurrentTime + ezMath::Max(data.m_Interval, deltaTime) + GetRandomTimeJitter(i, m_uiSeed);
          data.m_LastScheduledTime = m_CurrentTime;
        }

        m_ScheduledWork.PushBack(it);
      }
    }

    // re-sort
    for (auto& it : m_ScheduledWork)
    {
      if (it.Value().IsValid())
      {
        // make a copy of data and re-insert at new due time
        Data data = it.Value();
        m_WorkIdToData[data.m_Work] = InsertData(data);
      }

      m_Data.Remove(it);
    }
    m_ScheduledWork.Clear();
  }
}

template <typename T>
EZ_FORCE_INLINE typename ezIntervalScheduler<T>::DataMap::Iterator ezIntervalScheduler<T>::InsertData(Data& data)
{
  // make sure that we have a unique due time since the map can't store multiple keys with the same value
  int pos = 0;
  while (m_Data.Contains(data.m_DueTime))
  {
    data.m_DueTime += GetRandomTimeJitter(pos++, m_uiSeed);
  }

  return m_Data.Insert(data.m_DueTime, data);
}
