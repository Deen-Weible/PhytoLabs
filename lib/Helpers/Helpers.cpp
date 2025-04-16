int Wrap(int kX, int const kLowerBound, int const kUpperBound) {
  int range_size = kUpperBound - kLowerBound + 1;

  if (kX < kLowerBound)
    kX += range_size * ((kLowerBound - kX) / range_size + 1);

  return kLowerBound + (kX - kLowerBound) % range_size;
}

int SecondsToMinutes(int seconds) { return Wrap((seconds / 60), 0, 59); }

int SecondsToHours(int seconds) { return Wrap((seconds / 3600), 0, 23); }

int WrapSeconds(int seconds) { return Wrap(seconds, 0, 59); }

int Clamp(int value, int lower_bound, int upper_bound) {
  if (value > upper_bound)
    return upper_bound;
  else if (value < lower_bound)
    return lower_bound;
  else
    return value;
}