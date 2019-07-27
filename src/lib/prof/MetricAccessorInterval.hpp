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


/*
 * A MetricInterval (a,b) is a set of integer index values i
 * satisfying a <= i < b.
 */
typedef std::pair<unsigned int, unsigned int> MetricInterval;

/*
 * A MI_Vec is a MetricInterval representing a set of indices a <= i <
 * b and a vector of b-a doubles.
 */
typedef pair<MetricInterval, vector<double> > MI_Vec;

/*
 * One MI_Vec is less than another if all indices of one are less than
 * all indices of the other.
 */
class MI_Vec_Compare {
public:
  bool operator()(const MI_Vec &lhs, const MI_Vec &rhs) const {
    return lhs.first.second <= rhs.first.first;
  }
};

/*
 * MetricAccessorInterval represents a sparse vector as an ordered set
 * of MI_Vecs, along with a special 'cached' entry that is exposed to
 * the outside so that it can be written to.
 */
class MetricAccessorInterval : public MetricAccessor {
private:
  set<MI_Vec, MI_Vec_Compare> table; // The ordered set of MI_Vecs
  set<MI_Vec>::iterator cacheIter;   // The most recently accessed MI_Vec
  double cacheVal;		     // The value of the most recently
				     // accessed element of cacheIter.
  unsigned int cacheItem;	     // The index of the most recently
				     // accessed element of cacheIter.
  unsigned int nzCount;		     // The number of nonzeroes, not
				     // counting the cached element.
  unsigned int shiftCount;	     // An offset that is added to
				     // index values passed as
				     // parameters to produce the
				     // index value used for lookup in
				     // the set of MI_Vecs.  It is
				     // initially very large, which
				     // allows components of the
				     // sparse vector to be virtually
				     // shifted right by a very large
				     // amount.

  /*
   * flush - update the set of MI_Vecs to reflect changes made to
   * cacheVal, so that another element can become the one cached.
   */
  void flush(void) {
    /* Update nzCount to reflect the element being uncached. */
    nzCount += (cacheVal != 0);
    if (cacheIter != table.end()) {
      /*
       * A value already represented in an MI_Vec has changed.  C++
       * standard sets do not allow a simple modification-in-place, so
       * we must make a new MI_Vec to reflect the changed value and
       * replace the old one with the new one in the table.
       */
      MI_Vec copy(*cacheIter);
      copy.second[cacheItem - cacheIter->first.first] = cacheVal;
      table.erase(cacheIter++);
      cacheIter = table.insert(cacheIter, copy);
      return;
    }
    /*
     * If cacheVal is 0, there's no need to update the implicit zero
     * we're already representing by its absence.
     */
    if (cacheVal == 0)
      return;
#if 0
    std::cout << "Start: ";
    dump();
#endif
    /*
     * Make a new 1-element MI_Vec to represent just this value.
     */
    MetricInterval ival;
    vector<double> val;
    ival = make_pair(cacheItem, cacheItem+1);
    val = vector<double>(1, cacheVal);
    cacheIter = table.insert(cacheIter, make_pair(ival, val));

    /*
     * If this 1-element MI_Vec is immediately followed by another
     * MI_Vec, merge that one into this one, and remove that one from
     * the set.
     */
    set<MI_Vec>::iterator nextIter = next(cacheIter);
    if (table.end() != nextIter &&
	nextIter->first.first == cacheItem+1) {
      ival.second = nextIter->first.second;
      val.insert(val.end(),
		 nextIter->second.begin(), nextIter->second.end());
      table.erase(nextIter);
    }
    /*
     * If this MI_Vec is immediately preceded by another MI_Vec, merge
     * that one into this one, and remove that one from the set.
     */
    set<MI_Vec>::iterator prevIter;
    if (table.begin() != cacheIter &&
	(prevIter = prev(cacheIter))->first.second == cacheItem) {
      ival.first = prevIter->first.first;
      val.insert(val.begin(),
		 prevIter->second.begin(), prevIter->second.end());
      table.erase(prevIter);
    }
    /*
     * If we've grown the original 1-element MI_Vec since we added it
     * to the set, erase the old copy and re-insert the new, bigger
     * one.
     */
    if (ival.first + 1 != ival.second) {
      table.erase(cacheIter++);
      cacheIter = table.insert(cacheIter, make_pair(ival, val));
    }
#if 0
    std::cout << "End: ";
    dump();
#endif
  }

  /*
   * lookup - find the MI_Vec, if any, that includes the given index.
   * Update cache information with new value.
   */
  double lookup(unsigned int mId) {
    vector<double> dummy;
    /*
     * Make a pair to be a search key.
     */
    MI_Vec key = make_pair(make_pair(mId, mId+1), dummy);
    set<MI_Vec>::iterator it = table.find(key);
    cacheIter = it;
    cacheItem = mId;
    if (it == table.end())
      return 0;			// mId is beyond the last nonzero value
    unsigned int lo = it->first.first;
    unsigned int hi = it->first.second;
    if (lo <= mId && mId < hi)
      return it->second[mId - lo]; // mId is represented in some MI_Vec.
    return 0;			   // mId falls before any MI_Vecs, or
				   // between a pair.
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

    /*
     * shift_indices() - Subseqently, the values considered to be a
     * positions 0, 1, 2, ... will be considered to be at positions
     * shiftSize, shiftSize+1, shiftSize+2, ...
     */
  virtual void shift_indices(int shiftSize) {
    shiftCount -= shiftSize;
  }

  /*
   * idx() - provide a mutable reference value so that the caller can
   * modify the lvalue received and have those modifications reflected
   * in the set of MI_Vecs later.
   */
  virtual double &idx(unsigned int mId, unsigned int size = 0) {
    mId += shiftCount;
    if (cacheItem == mId)
       return cacheVal;
    /*
     *  The cached value is not the one requested, so flush the cache
     *  are fetch what was requested.
     */
    flush();
    cacheVal = lookup(mId);
    if (cacheVal != 0)
      --nzCount;
    return cacheVal;
  }

  /*
   * c_idx() - provide a value that the caller cannot modify, in a way
   * that does not modify this object.
   */
  virtual double c_idx(unsigned int mId) const {
    mId += shiftCount;
    if (cacheItem == mId)
      return cacheVal;
    vector<double> dummy;
    MI_Vec key = make_pair(make_pair(mId, mId+1), dummy);
    set<MI_Vec>::iterator it = table.find(key);
    if (it == table.end())
      return 0;			// index beyond last nonzero.
    unsigned int lo = it->first.first;
    unsigned int hi = it->first.second;
    if (lo <= mId && mId < hi)
      return it->second[mId - lo]; // index in some MI_Vec range.
    return 0;
  }

  /*
   * idx_ge() - provide the smallest index >= mId that is represented
   * by some MI_Vec, or UINT_MAX if there is none.  This allows
   * iteration over the elements of a vector without stopping to check
   * the ones that are obviously zero because they have no associated
   * MI_Vec.
   */
  virtual unsigned idx_ge(unsigned mId) const {
    mId += shiftCount;
    vector<double> dummy;
    MI_Vec key = make_pair(make_pair(mId, mId+1), dummy);
    set<MI_Vec>::iterator it = table.lower_bound(key);
    if (it == table.end()) {
      /* There is no MI_Vec with an index >= mId.  */
      if (mId <= cacheItem && cacheVal != 0.)
	/* There is, however, a cached nonzero value not yet added to
	   the MI_Vec set. */
	return cacheItem - shiftCount;
      /* All out of nonzeros. */
      return UINT_MAX;
    }
    unsigned int lo = it->first.first;
    if (mId <= cacheItem && cacheVal != 0. && cacheItem < lo)
      /*
       * There is a cached nonzero value not yet added to the MI_Vec
       * set before the one we've looked up.
       */
      return cacheItem - shiftCount;
    if (mId < lo)
      /*
       * mId falls between MI_Vecs, so jump to the start of the next one.
       */
      return lo - shiftCount;
    /*
     * mId falls within an MI_Vec, so just return it.
     */
    return mId - shiftCount;
  }

  /*
   * empty() - Are there any nonzeroes here?
   */
  virtual bool empty(void) const {
    return (nzCount == 0 && cacheVal == 0);
  }

#include <iostream>
  /*
   * dump() - A debugging artifact. Write a representation of the
   * values and their positions.
   */
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
