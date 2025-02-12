<script setup lang="ts">
import { ref, onMounted } from 'vue';

const formattedDate = ref('');

onMounted(() => {
  const currentDate = new Date();
  formattedDate.value = currentDate.toLocaleDateString('de-DE', {
    day: '2-digit',
    month: 'long',
    year: 'numeric'
  });
});
</script>

<template>
    <span id="date">{{ formattedDate }}</span>
</template>

