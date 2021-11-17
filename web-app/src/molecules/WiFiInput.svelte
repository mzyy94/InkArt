<script context="module" lang="ts">
  export type SubmitEvent = CustomEvent<{ ssid: string; password: string }>;
</script>

<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import { TextField } from "smelte";
  import ProgressButton from "../atoms/ProgressButton.svelte";

  const dispatch = createEventDispatcher();

  function submit() {
    dispatch("submit", {
      ssid,
      password,
    });
  }

  export let ssid: string;
  export let password: string;
  export let button = "Connect";
  export let pending = false;
</script>

<form>
  <TextField label="SSID" bind:value={ssid} />
  <TextField type="password" label="Password" bind:value={password} />
  <ProgressButton on:click={submit} loading={pending}>
    {button}
    <span slot="loading">Processing...</span>
  </ProgressButton>
</form>
