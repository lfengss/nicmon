/*
 * nicmon.c - Network Interface Card Monitor
 * 
 * A lightweight Windows utility for real-time NIC performance monitoring.
 * Uses NDIS OID queries and GetIfEntry2 for adapter statistics.
 */

#include <windows.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

#define POLL_INTERVAL_MS   1000
#define MAX_ADAPTERS       32
#define ADAPTER_NAME_LEN   256

typedef struct {
    wchar_t name[ADAPTER_NAME_LEN];
    char desc[ADAPTER_NAME_LEN];
    ULONG ifIndex;
    /* cumulative counters */
    ULONG64 rxBytes, txBytes;
    ULONG64 rxPackets, txPackets;
    ULONG rxErrors, txErrors;
    ULONG rxDropped, txDropped;
    /* derived (per interval) */
    double rxBps, txBps;
    double rxPps, txPps;
} AdapterStats;

typedef struct {
    AdapterStats adapters[MAX_ADAPTERS];
    int count;
} MonitorState;

static void QueryAdapters(MonitorState *state)
{
    ULONG bufLen = sizeof(MIB_IFTABLE) * MAX_ADAPTERS;
    MIB_IFTABLE *table = malloc(bufLen);
    
    if (GetIfTable(table, &bufLen, FALSE) == NO_ERROR) {
        int idx = 0;
        for (DWORD i = 0; i < table->dwNumEntries && idx < MAX_ADAPTERS; i++) {
            MIB_IFROW *row = &table->table[i];
            if (row->dwType != IF_TYPE_ETHERNET_CSMACD && row->dwType != IF_TYPE_IEEE80211)
                continue;
            
            AdapterStats *a = &state->adapters[idx++];
            a->ifIndex = row->dwIndex;
            wcscpy_s(a->name, ADAPTER_NAME_LEN, row->wszName);
            WideCharToMultiByte(CP_UTF8, 0, row->wszName, -1, a->desc, ADAPTER_NAME_LEN, NULL, NULL);
            
            /* Store cumulative for delta calculation */
            ULONG64 prevRx = a->rxBytes, prevTx = a->txBytes;
            a->rxBytes = row->dwInOctets;
            a->txBytes = row->dwOutOctets;
            a->rxPackets = row->dwInUcastPkts + row->dwInNUcastPkts;
            a->txPackets = row->dwOutUcastPkts + row->dwOutNUcastPkts;
            a->rxErrors = row->dwInErrors;
            a->txErrors = row->dwOutErrors;
            a->rxDropped = row->dwInDiscards;
            a->txDropped = row->dwOutDiscards;
            
            /* Derive per-second rates */
            a->rxBps = (double)(a->rxBytes - prevRx) * (1000.0 / POLL_INTERVAL_MS);
            a->txBps = (double)(a->txBytes - prevTx) * (1000.0 / POLL_INTERVAL_MS);
            a->rxPps = (double)(a->rxPackets) * (1000.0 / POLL_INTERVAL_MS);
        }
        state->count = idx;
    }
    free(table);
}

static void PrintHeader(void)
{
    printf("%-20s %12s %12s %10s %10s %8s %8s\n",
           "Adapter", "RX KB/s", "TX KB/s", "RX pkt/s", "TX pkt/s", "RX err", "TX err");
    printf("%-20s %12s %12s %10s %10s %8s %8s\n",
           "-------", "------", "------", "--------", "--------", "------", "------");
}

static void PrintStats(MonitorState *state)
{
    for (int i = 0; i < state->count; i++) {
        AdapterStats *a = &state->adapters[i];
        char shortName[32];
        /* Extract short name from long NDIS name */
        char *p = strrchr(a->desc, '_');
        strncpy_s(shortName, sizeof(shortName), p ? p + 1 : a->desc, _TRUNCATE);
        
        printf("%-20s %12.1f %12.1f %10.0f %10.0f %8lu %8lu\n",
               shortName,
               a->rxBps / 1024.0, a->txBps / 1024.0,
               a->rxPps, a->txPps,
               a->rxErrors, a->txErrors);
    }
}

int main(int argc, char *argv[])
{
    MonitorState state = {0};
    
    printf("NICMon v1.0.0 - Network Interface Monitor\n");
    printf("Polling every %d ms. Press Ctrl+C to stop.\n\n", POLL_INTERVAL_MS);
    
    /* Initial query to populate baseline */
    QueryAdapters(&state);
    PrintHeader();
    
    for (;;) {
        Sleep(POLL_INTERVAL_MS);
        QueryAdapters(&state);
        PrintStats(&state);
    }
    
    return 0;
}