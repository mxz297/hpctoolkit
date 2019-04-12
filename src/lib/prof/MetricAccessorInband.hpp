#ifndef __MetricAccessorInband_hpp__
#define __MetricAccessorInband_hpp__

#include "MetricAccessor.hpp"


class MetricAccessorInband : public MetricAccessor {
public:
  MetricAccessorInband(MetricAccessor *_mdata) : mdata(_mdata) {}
  virtual void shift_indices(int shiftSize) {
    return mdata->shift_indices(shiftSize);
  }
  virtual double &idx(unsigned int mId, unsigned int size = 0) {
    return mdata->idx(mId, size);
  }
  virtual double c_idx(unsigned int mId) const {
    return mdata->c_idx(mId);
  }
  virtual unsigned int idx_ge(unsigned int mId) const {
    return mId;
  }
  virtual bool empty(void) const {
    return mdata->empty();
  }
private:
  MetricAccessor *mdata;
};

#endif
