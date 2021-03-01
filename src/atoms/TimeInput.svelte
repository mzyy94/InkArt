<script lang="ts">
  import { createEventDispatcher } from "svelte";
  import { TextField } from "smelte";

  export let value: Date;
  $: time = value.toLocaleTimeString("en-US", { hour12: false });
  const dispatch = createEventDispatcher();
  let inputError = "";

  function inputTime(e: Event) {
    const input = e.target as HTMLInputElement;
    const pattern = new RegExp(input.pattern);

    if (pattern.test(input.value)) {
      inputError = "";
      const [hh, mm, ss] =
        input.value.split(":")?.map((t) => parseInt(t, 10)) ?? [];
      value.setHours(hh, mm, ss);
      dispatch("update", value);
    } else {
      inputError = "Invalid time format.";
    }
  }
</script>

<TextField
  value={time}
  error={inputError}
  on:change={inputTime}
  append="access_time"
  label="Time"
  pattern="^\d\d:\d\d:\d\d$"
/>
