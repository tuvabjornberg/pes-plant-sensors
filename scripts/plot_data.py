import matplotlib.pyplot as plt
from sys import argv

log_data = open(argv[1]).read()

timestamps = []
temperatures = []
humidities = []
moistures = []
lights = []

for line in log_data.strip().split("\n"):
    if not line.strip():
        continue

    parts = line.split("->")
    if len(parts) != 2:
        continue

    current_time = parts[0].strip()[:5]
    message = parts[1].strip()

    if "--- Periodic Sensor Read ---" in message:
        timestamps.append(current_time)
    elif message.startswith("Temp:"):
        temperatures.append(float(message.split(":")[1].strip()))
    elif message.startswith("Humidity:"):
        humidities.append(float(message.split(":")[1].strip()))
    elif message.startswith("Moisture:"):
        moistures.append(float(message.split(":")[1].strip()))
    elif message.startswith("Light:"):
        lights.append(float(message.split(":")[1].strip()))


fig, axs = plt.subplots(4, 1, sharex=True, figsize=(18, 8))
dislodge_index = 16


seri = [temperatures, humidities, moistures, lights]
ylabel = [
    "Temperature [$^\\circ$C]",
    "Relative Humidity [%]",
    "Soil Moistures",
    "Light level [lx]",
]

colors = ["b", "g", "r", "purple"]

for i, (ax, series, ylabel, color) in enumerate(zip(axs, seri, ylabel, colors)):
    ax.plot(series, "-o", color=color, label=ylabel)
    ax.set_ylabel(ylabel)
    ax.axvline(
        8 + 0.5,
        linestyle="--",
        color="blue",
        label="Watered" if i == 3 else None,
    )

axs[0].set_ybound(17.5, 22.5)
axs[3].set_xbound(-0.5, len(timestamps) - 0.5)
axs[3].set_xlabel("Time")
axs[3].set_xticks(range(len(timestamps)))
axs[3].set_xticklabels(timestamps, rotation=45)  # , ha="right")

fig.legend(loc="upper center", ncol=5, bbox_to_anchor=(0.5, 1.02))
plt.tight_layout()
plt.savefig("during_watering.png", bbox_inches="tight")
