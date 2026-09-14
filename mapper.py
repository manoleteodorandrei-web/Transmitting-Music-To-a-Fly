import pandas as pd
import numpy as np

print("Loading raw dataset...")
df_conn = pd.read_csv("connections.csv")

print("Mapping 64-bit IDs...")
unique_ids = pd.concat([df_conn['pre_root_id'], df_conn['post_root_id']]).unique()
id_mapping = {old_id: new_idx for new_idx, old_id in enumerate(unique_ids)}

clean_df = pd.DataFrame()
clean_df['source'] = df_conn['pre_root_id'].map(id_mapping)
clean_df['target'] = df_conn['post_root_id'].map(id_mapping)

# Biological Restoration: GABA is inhibitory (negative), others are excitatory (positive)
clean_df['weight'] = np.where(df_conn['nt_type'] == 'GABA', 
                              -df_conn['syn_count'].astype(float) / 25.0, 
                              df_conn['syn_count'].astype(float) / 25.0)

clean_df.to_csv("mapped_connections.csv", index=False)
print(f"Saved {len(clean_df)} pure biological synapses to mapped_connections.csv!")