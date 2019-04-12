#ifndef __MetricAccessorInterval_hpp__
#define __MetricAccessorInterval_hpp__

#include "MetricAccessor.hpp"
#include "Metric-IData.hpp"
#include <set>
#include <utility>
#include <vector>
using std::make_pair;
using std::pair;
using std::set;
using std::vector;

typedef std::pair<unsigned int, unsigned int> MetricInterval;
typedef pair<MetricInterval, vector<double> > MI_Vec;

class MI_Vec_Compare {
public:
  bool operator()(const MI_Vec &lhs, const MI_Vec &rhs) const {
    return lhs.first.second <= rhs.first.first;
  }
};

class MetricAccessorInterval : public MetricAccessor {
private:
  set<MI_Vec, MI_Vec_Compare> table;
  set<MI_Vec>::iterator cacheIter;
  double cacheVal;
  unsigned int cacheItem;
  unsigned int nzCount;
  unsigned int shiftCount;

  void flush(void) {
    nzCount += (cacheVal != 0);
    if (cacheIter != table.end()) {
      MI_Vec copy(*cacheIter);
      copy.second[cacheItem - cacheIter->first.first] = cacheVal;
      table.erase(cacheIter++);
      cacheIter = table.insert(cacheIter, copy);
      return;
    }
    if (cacheVal == 0)
      return;
    std::cout << "Start: ";
    dump();
    MetricInterval ival;
    vector<double> val;
    ival = make_pair(cacheItem, cacheItem+1);
    val = vector<double>(1, cacheVal);
    cacheIter = table.insert(cacheIter, make_pair(ival, val));
    set<MI_Vec>::iterator nextIter = next(cacheIter);
    if (table.end() != nextIter &&
	nextIter->first.first == cacheItem+1) {
      ival.second = nextIter->first.second;
      val.insert(val.end(),
		 nextIter->second.begin(), nextIter->second.end());
      table.erase(nextIter);
    }
    set<MI_Vec>::iterator prevIter;
    if (table.begin() != cacheIter &&
	(prevIter = prev(cacheIter))->first.second == cacheItem) {
      ival.first = prevIter->first.first;
      val.insert(val.begin(),
		 prevIter->second.begin(), prevIter->second.end());
      table.erase(prevIter);
    }
    if (ival.first + 1 != ival.second) {
      table.erase(cacheIter++);
      cacheIter = table.insert(cacheIter, make_pair(ival, val));
    }
    std::cout << "End: ";
    dump();
  }

  double lookup(unsigned int mId) {
    vector<double> dummy;
    MI_Vec key = make_pair(make_pair(mId, mId+1), dummy);
    set<MI_Vec>::iterator it = table.find(key);
    cacheIter = it;
    cacheItem = mId;
    if (it == table.end())
      return 0;
    unsigned int lo = it->first.first;
    unsigned int hi = it->first.second;
    if (lo <= mId && mId < hi)
      return it->second[mId - lo];
    return 0;
  }
 
public:
  MetricAccessorInterval(void):
    table(), cacheIter(table.end()), cacheVal(0.), cacheItem(-1), nzCount(0), shiftCount(UINT_MAX/2)
  {
  }

  MetricAccessorInterval(const MetricAccessorInterval &src):
    table(src.table), cacheIter(table.end()), cacheVal(src.cacheVal),
    cacheItem(src.cacheItem), nzCount(src.nzCount), shiftCount(src.shiftCount)
  {
  }

  MetricAccessorInterval(Prof::Metric::IData &_mdata):
    table(), cacheIter(table.end()), cacheVal(0.), cacheItem(-1) , nzCount(0), shiftCount(UINT_MAX/2)
  {
    for (unsigned i = 0; i < _mdata.numMetrics(); ++i) {
      idx(i) = _mdata.metric(i);
      if (_mdata.metric(i) != 0)
	++nzCount;
    }
  }

  virtual void shift_indices(int shiftSize) {
    shiftCount -= shiftSize;
  }

  virtual double &idx(unsigned int mId, unsigned int size = 0) {
    mId += shiftCount;
    if (cacheItem == mId)
       return cacheVal;
    flush();
    cacheVal = lookup(mId);
    if (cacheVal != 0)
      --nzCount;
    return cacheVal;
  }

  virtual double c_idx(unsigned int mId) const {
    mId += shiftCount;
    if (cacheItem == mId)
      return cacheVal;
    vector<double> dummy;
    MI_Vec key = make_pair(make_pair(mId, mId+1), dummy);
    set<MI_Vec>::iterator it = table.find(key);
    if (it == table.end())
      return 0;
    unsigned int lo = it->first.first;
    unsigned int hi = it->first.second;
    if (lo <= mId && mId < hi)
      return it->second[mId - lo];
    return 0;
  }

  virtual unsigned idx_ge(unsigned mId) const {
    mId += shiftCount;
    vector<double> dummy;
    MI_Vec key = make_pair(make_pair(mId, mId+1), dummy);
    set<MI_Vec>::iterator it = table.lower_bound(key);
    if (it == table.end()) {
      if (mId <= cacheItem && cacheVal != 0.)
	return cacheItem - shiftCount;
      return UINT_MAX;
    }
    unsigned int lo = it->first.first;
    if (mId <= cacheItem && cacheVal != 0. && cacheItem < lo)
      return cacheItem - shiftCount;
    if (mId < lo)
      return lo - shiftCount;
    return mId - shiftCount;
  }

  virtual bool empty(void) const {
    return (nzCount == 0 && cacheVal == 0);
  }

#include <iostream>
  void dump(void)
  {
    std::cout << "Cache[" << cacheItem - shiftCount << "] = " << cacheVal << "\n";
    for (set<MI_Vec>::iterator it = table.begin(); it != table.end(); it++) {
      std::cout << "[" << it->first.first - shiftCount << ", " << it->first.second - shiftCount << "):";
      for (vector<double>::const_iterator i = it->second.begin(); i != it->second.end(); i++)
	std::cout << " " << *i;
      std::cout << "\n";
    }
  }

};

#endif
