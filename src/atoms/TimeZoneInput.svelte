<script lang="ts">
  import { TextField } from "smelte";

  export let timeZone: string;

  let inputError = "";

  function inputTimeZone(e: Event) {
    const input = e.target as HTMLInputElement;
    try {
      new Intl.DateTimeFormat("default", { timeZone: input.value });
      inputError = "";
      timeZone = input.value;
    } catch {
      inputError = "Invalid timezone.";
    }
  }
</script>

<TextField
  value={timeZone}
  error={inputError}
  on:change={inputTimeZone}
  append="public"
  label="Time zone"
/>
