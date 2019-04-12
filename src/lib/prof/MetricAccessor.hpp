#ifndef __MetricAccessor_hpp__
#define __MetricAccessor_hpp__

class MetricAccessor {
public:
  virtual ~MetricAccessor() {}; 
  virtual void shift_indices(int shiftSize) = 0;
  virtual double &idx(unsigned int mId, unsigned int size = 0) = 0;
  virtual double c_idx(unsigned int mId) const = 0;
  virtual unsigned int idx_ge(unsigned int mId) const = 0;
  virtual bool empty(void) const = 0;
};

#endif

