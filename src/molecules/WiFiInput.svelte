<script context="module" lang="ts">
  export type SubmitEvent = CustomEvent<{ ssid: string; password: string }>;
</script>

<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import { Button, ProgressCircular, TextField } from "smelte";

  const dispatch = createEventDispatcher();

  function submit() {
    dispatch("submit", {
      ssid,
      password,
    });
  }

  let clazz = "";
  export { clazz as class };
  export let ssid: string;
  export let password: string;
  export let button = "Connect";
  export let pending = false;
</script>

<form class={clazz}>
  <TextField label="SSID" bind:value={ssid} />
  <TextField type="password" label="Password" bind:value={password} />
  <Button class="w-full" on:click={submit} disabled={pending}>
    {#if pending}
      <span class="inline-block align-text-bottom">
        <ProgressCircular size={16} width={2} color="secondary" />
      </span> Processing...
    {:else}
      {button}
    {/if}
  </Button>
</form>
