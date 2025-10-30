import pandas as pd
import matplotlib.pyplot as plt

# Load CSV
df = pd.read_csv("siege_results.csv")

# Plot Transaction Rate
plt.figure()
plt.plot(df["Concurrency"], df["TransactionRate"], marker="o")
plt.title("Webserv - Requests per Second vs Concurrency")
plt.xlabel("Concurrent Users")
plt.ylabel("Requests per Second")
plt.grid(True)
plt.savefig("webserv_throughput.png", dpi=200)

# Plot Response Time
plt.figure()
plt.plot(df["Concurrency"], df["ResponseTime"], marker="o", color="orange")
plt.title("Webserv - Response Time vs Concurrency")
plt.xlabel("Concurrent Users")
plt.ylabel("Average Response Time (s)")
plt.grid(True)
plt.savefig("webserv_latency.png", dpi=200)

print("✅ Charts saved as webserv_throughput.png and webserv_latency.png")
``
