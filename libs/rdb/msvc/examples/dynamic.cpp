void dynamic_demo(const string& first, const string& name) {
  person p;

  vector< dynamic_expression<void> > exprs;
  dynamic_expression<boolean> cond;

  string names[2];

  if (first.length()) {
    exprs.push_back(p.first);
    cond = p.first.like(first + "%");
  }

  if (name.length()) {
    exprs.push_back(p.name);
    cond = cond ? p.name.like(name + "%") : p.name.like(name + "%") && cond;
  }

  auto sel = select(p.id, exprs, p.age).from(p).where(pi.d != null && cond);
  auto res = db.execute(sel);

  int id = res.fetch_col<0>();
  int col = 1;

  if (first.length()) {
    res.fetch<1, string>(col++);
  }

  if (name.length()) {
    res.fetch<1, string>(col++);
  }

  int id = res.fetch_col<2>();

  res.fetch_col<2>(age);

  result_set res = db.execute(select(p.id, exprs, p.age).from(p).where(pi.d != null && cond))
    .bind<0>(id)
    .bind<1>(first.length() ? i++ : -1, r1)
    .bind<1>(name.length() ? i++ : -1, r2)
    .bind<2>(age)
    .iter();

  while (res()) {
    // ...
  }


}