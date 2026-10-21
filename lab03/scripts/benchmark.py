import csv
import shutil
import os
import platform
import random
import statistics
import subprocess
from pathlib import Path

root = Path(__file__).resolve().parents[1]
os.chdir(root)
out = root / "artifacts"
out.mkdir(exist_ok=True)
cases = [(10, 10000), (10, 1000000), (100, 1000000)]
thread_counts = [1, 2, 4, 8, 16]
jobs = [(k, n, p, repeat) for k, n in cases for p in thread_counts for repeat in range(5)]
random.Random(42).shuffle(jobs)
rows = []
counts = {}
subprocess.run(["./build/dice", "--quiet", "--threads", "1", "10", "0", "0", "0", "10000"],
               check=True, stdout=subprocess.DEVNULL)
for k, n, p, repeat in jobs:
    text = subprocess.check_output(["./build/dice", "--quiet", "--threads", str(p),
                                    "--seed", "42", str(k), "0", "0", "0", str(n)], text=True)
    values = dict(line.split(": ", 1) for line in text.splitlines())
    result = tuple(int(values[key]) for key in ("Побед игрока 1", "Побед игрока 2", "Ничьих"))
    if (k, n) in counts:
        assert counts[k, n] == result
    counts[k, n] = result
    rows.append((k, n, p, repeat + 1, float(values["Время, с"])))
with (out / "benchmark_raw.csv").open("w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(["rounds", "experiments", "threads", "repeat", "seconds"])
    writer.writerows(sorted(rows))
summary = []
for k, n in cases:
    baseline = statistics.median(r[4] for r in rows if r[:3] == (k, n, 1))
    for p in thread_counts:
        times = [r[4] for r in rows if r[:3] == (k, n, p)]
        seconds = statistics.median(times)
        speedup = baseline / seconds
        summary.append((k, n, p, seconds, speedup, speedup / p, min(times), max(times)))
with (out / "benchmark.csv").open("w", newline="") as file:
    writer = csv.writer(file)
    writer.writerow(["rounds", "experiments", "threads", "median_seconds", "speedup", "efficiency", "min_seconds", "max_seconds"])
    writer.writerows(summary)
environment = platform.platform() + "\n" + subprocess.check_output(["lscpu"], text=True)
environment += "\n" + subprocess.check_output(["gcc", "--version"], text=True)
(out / "environment.txt").write_text(environment, encoding="utf-8")
lines = ["# Исследование ускорения и эффективности", "",
         "Замеры: WSL 2, Linux, GCC -O2; seed=42; начальные очки 0:0, завершено 0 туров.",
         "Для каждой конфигурации выполнено пять запусков, взята медиана. Порядок запусков перемешан.",
         "Время CLOCK_MONOTONIC включает выделение памяти, создание потоков, расчёт, join и объединение результатов.",
         "Диагностика PID/TID выключена. Результаты всех запусков одного набора данных совпали.", "",
         "S(p) = T(1) / T(p); E(p) = S(p) / p. p включает основной поток.", "",
         "| K | Опыты | p | Время, с | Ускорение S | Эффективность E | Мин.–макс., с |",
         "| --- | --- | --- | --- | --- | --- | --- |"]
for k, n, p, t, s, e, low, high in summary:
    lines.append(f"| {k} | {n} | {p} | {t:.6f} | {s:.3f} | {e:.3f} | {low:.6f}–{high:.6f} |")
lines += ["", "Для коротких запусков создание потоков и планирование могут занимать больше времени, чем сама работа.",
          "При увеличении числа опытов или оставшихся туров растёт объём независимых вычислений, на котором можно окупить эти затраты.",
          "Ускорение ограничивают последовательные этапы, частота процессора, общие ресурсы ядер и планирование WSL/Windows.",
          "16 логических процессоров не означают 16 независимых физических ядер. Фоновая нагрузка и нагрев влияют на время.",
          "Выводы относятся к этой машине и этим входным данным; замеры не гарантируют ускорения на другой системе.",
          "Увеличение числа потоков при неизменном числе опытов не повышает точность оценки вероятностей.",
          "Вероятности оценены частотами; типичная статистическая погрешность уменьшается как 1/sqrt(N).", "",
          "Если E>1, отношение медиан само по себе не доказывает сверхлинейное ускорение: сравните диапазоны времени и влияние нагрузки.",
          "Версия исходных измерений и описания среды для отчёта также сохраняется в report/measurements.", ""]
(root / "BENCHMARK.md").write_text("\n".join(lines), encoding="utf-8")
archive = root / "report/measurements"
archive.mkdir(parents=True, exist_ok=True)
for name in ["benchmark_raw.csv", "benchmark.csv", "environment.txt"]:
    shutil.copy2(out / name, archive / name)
print("Сохранены BENCHMARK.md, artifacts/benchmark*.csv и artifacts/environment.txt")
